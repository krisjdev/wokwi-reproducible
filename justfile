default:
    just -v

test *flags:
    pytest {{flags}} -s tests/

ww-test:
    wokwi-cli --scenario tests/test_touch.yaml