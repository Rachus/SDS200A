# Configuration

The device is configured using control transfers.

## Typical startup sequence

The collected traces of the device's communication with the SoftScope2 software under Windows along with experimentation suggests the following steps to communicate with the device

1. [Reset](#reset) the device
2. [Reset relays](#relays---attenuation-and-coupling)
3. *Optional:* Read [eeprom data](devicedata.md#0xc7---read-from-eeprom) and use [0xc5](devicedata.md#0x5---unknown)
4. [Reset](#reset) the device (sic!)
5. [Set relays](#relays---attenuation-and-coupling)
6. Set the [calibration](lib/readme.md#calibration) [offset](#offset)
7. Build the [statusword for time/div](#time-div)
8. Modify the [statusword with the trigger settings](#trigger)
9. Set the [trigger offset](#trigger-offset)
10. [Acquire data](dataformat.md#data-aqcisition)



## Relays - Attenuation and Coupling
If you look at the semantics available via FCC (see [resources](resources.md)), the device possesses six relays, three per channel. Those relays are responsible for the *Attenuation* and *Coupling* settings of the device. Those relays can be set via the `0xb5` control requests:

| Type        | bmRequestType | bRequest | wValue | wIndex | wLength | Data    |
|-------------|---------------|----------|--------|--------|---------|---------|
| Control out | 0x40          | 0xb5     |      0 |      0 |       1 | payload |

The payload can be used to enable/disable the relays separately. Under Windows, the "SoftScope2" software sends two `0xb5`-requests per relay to set.
The first request ships the actual payload, the second request "flushes" the value.
*Important:* The software has to sleep between sending those requests, as the sds200a needs some time to apply the payload, and the `0x00` send afterwards overwrites this setting.

| Type        | bmRequestType | bRequest | wValue | wIndex | wLength | Data    |
|-------------|---------------|----------|--------|--------|---------|---------|
| Control out | 0x40          | 0xb5     |      0 |      0 |       1 | payload |
| Control out | 0x40          | 0xb5     |      0 |      0 |       1 |    0x00 |

The payload uses bitflags to activate or deactivate the relays.

### activating arrays
To activate relay x, one sends a `b5`-request with the x-bit set.
E.g. to set relay 5, the corresponding requests would be:

| Control out | 0x40          | 0xb5     |      0 |      0 |       1 |    0x10 |
| Control out | 0x40          | 0xb5     |      0 |      0 |       1 |    0x00 |

where 0x10 == 0b00010000

To unset the relay, the inverted mask has to be send. 
E.g. to reset relay 5, the corresponding requests would be:

| Control out | 0x40          | 0xb5     |      0 |      0 |       1 |    0x10 |
| Control out | 0x40          | 0xb5     |      0 |      0 |       1 |    0x00 |

where 0x10 == 0b11101111

### Relay functions
#### Channel 1:
| Function         | Relay number |
|------------------|--------------|
| Coupling         |            0 |
| 10V-Attenuation  |            1 |
| 100V-Attenuation |            2 |

#### Channel 2:
| Function         | Relay number |
|------------------|--------------|
| Coupling         |            3 |
| 10V-Attenuation  |            4 |
| 100V-Attenuation |            5 |

## Time/div

The SDS200A has two request with the codes `0xb1` and `0xb3`.
Those two values use some sort of statusword that is transferred to the device and which is responsible for at least the time/div and the trigger settings.

Even though both requests use the same statusword, `0xb3` seems to be responsible for the time/div setting and the `0xb1` request for the trigger setting.

To us, there was no clear structure in the time/div part of the statusword (maybe this word is used to actually drive an internal circuit), so the viable approach is to use the lookuptable below for each time/div and apply the modifications for the triggersettings afterwards.

*Note:* As it seems, one wants to send the same word to `0xb3` and `0xb1` for the device to apply the configuration

| Time/div | Statusword                                      |
|----------|-------------------------------------------------|
| 2ns      | 0e010000 08005000 00000100 00000000 00000000 01 |
| 4ns      | 0e010000 08005000 00000100 00000000 00000001 01 |
| 10ns     | 0e010000 08005100 00000100 00000000 00000002 01 |
| 20ns     | 0e010000 08005000 00000100 00000000 00000003 01 |
| 40ns     | 0e010000 08005000 00000100 00000000 00000004 01 |
| 100ns    | 0e010000 08005000 00000100 00000000 00000005 01 |
| 200ns    | 0e010000 08005000 00000100 00000000 00000006 01 |
| 400ns    | 0e010000 0e008000 00000100 00000000 00000007 01 |
| 1µs      | 0e010000 21001801 00000100 00000000 00000008 01 |
| 2µs      | 0e010000 40000e02 00000100 00000000 00000009 01 |
| 4µs      | 0e010000 7f000804 00000100 00000000 0000000a 01 |
| 10µs     | 0e010000 3a01de09 00000100 00000000 0000000b 01 |
| 20µs     | 1e030000 3a01dc09 00000100 00000000 0000000c 01 |
| 40µs     | 1e070000 3a01db09 00000100 00000000 0000000d 01 |
| 100µs    | 16090100 3a01db09 00000100 00000000 0000000e 01 |
| 200µs    | 16090300 3a01db09 00000100 00000000 0000000f 01 |
| 400µs    | 16090700 3a01da09 00000100 00000000 00000010 01 |
| 1ms      | 16091300 3a01da09 00000100 00000000 00000011 01 |
| 2ms      | 16092700 3a01da09 00000100 00000000 00000012 01 |
| 4ms      | 16094f00 3a01da09 00000100 00000000 00000013 01 |
| 10ms     | 1609c700 3a01da09 00000100 00000000 00000014 01 |
| 20ms     | 16098f01 3a01da09 00000100 00000000 00000015 01 |
| 40ms     | 16091f03 3a01da09 00000100 00000000 00000016 01 |
| 100ms    | 1609cf07 3a01da09 00000100 00000000 00000017 01 |
| 200ms    | 16099f0f 3a01da09 00000100 00000000 00000018 01 |
| 400ms    | 1613401f 3a01da09 00000100 00000000 00000019 01 |
| 1s       | 2e010000 00001000 00000500 00000000 0000009d 01 |
| 2s       | 2e010000 00001000 00000500 00000000 0000009e 01 |
| 4s       | 2e010000 00001000 00000500 00000000 0000009f 01 |
| 10s      | 2e010000 00001000 00000500 00000000 000000a0 01 |

The requests have the following structure:
| Type        | bmRequestType | bRequest | wValue | wIndex | wLength | Data       |
|-------------|---------------|----------|--------|--------|---------|------------|
| Control out | 0x40          | 0xb1     |      0 |      0 |      21 | statusword |
| Control out | 0x40          | 0xb3     |      0 |      0 |      21 | statusword |

## Trigger

### Basic settings
The statusword (see above) also does encode information on the triggersettings. Our traces and testing suggest the following bits to be responsible for the triggersettings:
```
       1        2        3        4         5       6         7        8
00010110 00001001 00000111 00000000  00111010 00000001 11011010 00001001


       9       10       11       12        13       14       15       16
00000000 00000000 00000001 000000XX  00000000 00000000 00000000 00000000
                                 ^^

      17       18       19       20        12
00000000 00000000 00000011 Y0010000  00000001
                           ^
```

The following table should map the most basic triggersettings (channel, normal/auto, rising/falling edges)
 to the necessary bit-settings

| Description        | XX | Y |
|--------------------|----|---|
| ch1 normal rising  | 00 | 1 |
| ch1 normal falling | 01 | 1 |
| ch2 normal rising  | 10 | 1 |
| ch2 normal falling | 11 | 1 |
| ch1 auto rising    | 00 | 0 |
| ch1 auto falling   | 01 | 0 |
| ch2 auto rising    | 10 | 0 |
| ch2 auto falling   | 11 | 0 |

### Horizontal trigger
There is a setting to select the position of the trigger-event within the data returned from the device. 
When changing this setting, the bits marked Z are changed (and therefore encode these settings).

```
       1        2        3        4         5        6        7        8
00010010 00001001 00000011 00000000  01110100 00000010 ZZZZZZ00 00ZZZZZZ
                                                       ^^^^^^     ^^^^^^

       9       10       11       12        13       14       15       16
00000000 00000000 00000001 00000001  00000000 00000000 00000000 00000000

      17       18       19       20        21
00000000 00000000 00000000 10010000  00000001
```

### Trigger offset
See [Offsets](#offset), as this is done in the same way (exept for a bit flag)

## Offset

To change voltage and trigger offsets, the `0xb2` request is used. It always has three bytes of payload.

A request consisting of `0xABCDEF` has the following meaning:
* The selected offset seems to `0xDAB`
* The last byte `0xF` has further status information:
	* If `0xF & 1` evaluates to true, channel 2 is selected, else channel 1 is selected (this is unfortunately unconfirmed)
	* If `0xF & 2` evaluates to true, the trigger offset is set, else the voltage offset is set.
* The remaining bytes (`0xC` and `0xE`) were always `0`. Therefore we assume they are not required.

## Reset

It seems that a request of type `0xd0` resets the oscilloscope. The Softscope software uses it when initializing. Since (at least some of) the relays are bistable it mostly is used in combination with switching the relays to a known position. Notably the reset request is set before and after resetting the relays.

## Unknown requests

The traces also had some requests that couldn't be affiliated to any
functionality of the device:
* `0xb6`: Occurs seldom
* `0xb4`: Occurs more often but sending different values did not change the
          behaviour of the device. It has one byte of payload.

