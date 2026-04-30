import asyncio
from pathlib import Path
import os
import pytest
from wokwi_client import WokwiClient, IdfFirmwareUploadResult

# get project root
current_dir = Path(__file__).parent
project_root = current_dir.parent

IMAGE_COUNTER = 0

async def take_screenshot(c: WokwiClient):
    global IMAGE_COUNTER
    img_path = Path(".") / "tests" / "frames" / f"pyclient_{IMAGE_COUNTER}.png"
    img = await c.save_framebuffer_png("esp", img_path)

    IMAGE_COUNTER += 1

@pytest.mark.asyncio
async def test_touch():
    serial_queue = asyncio.Queue()

    def serial_callback(data):
        serial_queue.put_nowait(data.decode('utf-8'))
        print(data.decode('utf-8'), end='')

    async def wait_for_serial(expected: str, timeout: int = 30, retries: int = 3):
        for attempt in range(retries):
            try:
                async with asyncio.timeout(timeout):
                    while True:
                        line = await serial_queue.get()
                        if expected in line:
                            break

                print(f"\033[92mFound expected serial: '{expected}'\033[0m")
                return True
            except TimeoutError:
                if attempt < retries - 1:
                    print("retrying...")

        print(f"\033[93mfailed to find '{expected}' in serial output\033[0m")
        return False


    token = os.environ.get("WOKWI_CLI_TOKEN")
    if not token:
        pytest.fail("WOKWI_CLI_TOKEN environment variable not found")

    client = WokwiClient(token)
    await client.connect()

    flasher_args_path = project_root / "build" / "flasher_args.json"
    await client.upload_file("diagram.json", project_root / "diagram.json")
    firmware: IdfFirmwareUploadResult = await client.upload_idf_firmware(flasher_args_path)

    await client.start_simulation(firmware=firmware.firmware, flash_size=firmware.flash_size)
    monitor_task = client.serial_monitor(serial_callback)

    try:
        await wait_for_serial("Returned from app_main()")
        await take_screenshot(client)

        button_clicked = False
        for i in range(3):
            print(f"attempt {i+1}")
            await client.touch_event("esp", x=160, y=220, event="press")
            if await wait_for_serial("app: button clicked!", timeout=10, retries=1):
                button_clicked = True
                break

        if not button_clicked:
            pytest.fail("button was never clicked after multiple retries")


    finally:
        monitor_task.cancel()
        await client.disconnect()
