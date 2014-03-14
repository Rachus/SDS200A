# Data

## Data acquisition
Data is received from the device through polling and read using USB Bulk transfers.

To determine, whether new data is available the vendor-specific 0xc0 control-request is used:

|------------+---------------+----------+--------+--------+---------|
| Type       | bmRequestType | bRequest | wValue | wIndex | wLength |
|------------+---------------+----------+--------+--------+---------|
| Control In | 0xc0          | 0xc0     | 0x0    | 0x0    |       1 |
|------------+---------------+----------+--------+--------+---------|

The returned value indicates how much information is available to be read (most likely in terms of bulk transfers, more on that later). A value > 0 means that new data is available.

If (and only if) the 0xc0-request indicated that information is available, data can be read via bulk read. Note that bulk-reads will otherwise timeout. The bulk-in is of the following format:
|---------+----------+-----------|
| Type    | Endpoint | Size      |
|---------+----------+-----------|
| Bulk in | 0x82     | see below |
|---------+----------+-----------|
The "size"-parameter seems to be linked to the current time/div setting. The Windows-Software that ships with the device uses more, smaller bulk reads for smaller time/div-values. As the size parameter is not communicated to the device, the matching settings have to be used.

TODO: list values here

## Data format

The data obtained by the bulk-in requests consists of two parts (display here in groups of four-byte-groups):

```
		XXXXXXXX XXXXXXXX DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA
		DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA
		DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA
		DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA
		DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA DATADATA
```
There structure and interpretation of the first eight bytes are unknown as time of writing, but do not seem to be necessarry for a correct interpretation of the measurement data.

### Measurement data
Each measurement (marked DATA above) is encoded using two bytes.
The actual data measurement-data is the raw 10bit unsigned value (most likely this is the raw value returned from the device's A/D-converter, which provides 10bit-values, see [docs](./resources.md)).

```
		   Low      High
 Binary:   00XXXXXX 0000YYYY
 Mask:     0x3f     0xf

      =>  YYYYXXXXXX, 10bit unsigned, range (0 -> 1023)
```

### Channel
The channels is most likely encoded in the second bit of the second byte:
```
		   00000000 0Z000000
		   Z = 1 => channel 2
		   Z = 0 => channel 1
	 Mask:          0x40
```
However, the same position in the first byte might be a candidate as well, both seemed accurate during our tests.

### Measurement validity

A sample should be valid, if the first bit of the second byte is set:
```
		   00000000 10000000
	 Mask:          0x80
```

However there are sporadical samples of the following style:
```
		   11111111 11111111
```

The fix for this that worked for as was to declare samples as invalid, if BB is not equal to 00
```
		   00000000 00BB0000
	 Mask:          0x30
```
The best guess seems to be that those samples represent the the `RIS_MISSING` values talked about in the [SDK Users Guide](http://www.softdsp.com/BIBoard/list.php?id=eng_download)
> "If RAW_DATA holds the value of [...] 1024 (for SDS 200A), these imply the special value as RIS_MISSING."
