# lua4882
Access to NI's NI-488.2 (GPIB) driver

## API

The following commands (mostly traditional NI-488.2 calls) are implemented:

### ibclr()

Purpose: Clear a specific device.

```lua
local gpib = require "lua4882"

-- Returns content of IBSTA as table and an error message
local stat, errmsg = gpib.ibclr(3)	-- clears device 3

-- On success:
stat = <STATUS_TABLE>	-- see below
errmsg = nil
-- On failure:
handle = <STATUS_TABLE>	-- see below
errmsg = "Error code and detailed description"

-- Example with no GPIO-adapter attached, therefore ERR = true and errmsg != nil
for i,v in pairs(stat) do print(i,v) end
RQS     false
LACS    false
END     false
CMPL    false
TIMO    false
ERR     true
DTAS    false
TACS    false
LOK     false
REM     false
CIC     false
SRQI    false
DCAS    false
ATN     false

print(errmsg)
EHDL: The input handle is invalid
```

For the meaning of the status bits see https://documentation.help/NI-488.2/gpib2o8j.html.

### ibdev()

Purpose: Open and initialize a GPIB device descriptor.

```lua
local gpib = require "lua4882"
local boardIndex = 0	-- Commonly used index of bus controller
local primaryAddr = 1	-- Primary GPIB address of remote device
local secondaryAddr = 0	-- No secondary GPIB address of remote device
local timeout = 11		-- Timeout 1 s (see table below)
local eoiMode = 1		-- Asserts GPIB End-or-Identify (EOI) line at end of transfer
local eosMode = 0		-- No end-of-string character

-- Returns device handle and error message
local handle, errmsg = gpib.ibdev(boardIndex, primaryAddr, secondaryAddr, timeout, eoiMode, eosMode)

-- On success:
handle = <DEVICE_HANDLE>
errmsg = nil
-- On failure:
handle = nil
errmsg = "Error code and detailed description"

-- Interactive example (without an GPIB adapter installed)
Lua 5.4.7  Copyright (C) 1994-2024 Lua.org, PUC-Rio
> gpib=require "lua4882"
> gpib.ibdev(0,3,0,3,1,0)
nil     ENEB: Non-existent interface board
```

The following timeout index values may be used.

| Index   | 0    | 1     | 2     | 3      | 4      | 5    | 6    | 7     | 8     |
| ------- | ---- | ----- | ----- | ------ | ------ | ---- | ---- | ----- | ----- |
| Timeout | none | 10 µs | 30 µs | 100 µs | 300 µs | 1 ms | 3 ms | 10 ms | 30 ms |

| Index   | 9      | 10     | 11   | 12   | 13   | 14   | 15    | 16    | 17     |
| ------- | ------ | ------ | ---- | ---- | ---- | ---- | ----- | ----- | ------ |
| Timeout | 100 ms | 300 ms | 1 s  | 3 s  | 10 s | 30 s | 100 s | 300 s | 1000 s |

### ibrd()

Purpose: Read data from a device into a user buffer. Data may be read as ASCII-string, as table of single ASCII-characters, or as table of raw binary data.

```lua
local gpib = require "lua4882"

-- Example 1: read 16 bytes from device 3 as ASCII and returns result as string
local data, stat, errmsg = gpib.ibrd(3,16)
-- On success:
data = "<SOME_ASCII_STRING>"
stat = <STATUS_TABLE>	-- see description for ibclr()
errmsg = nil	-- no error message
-- On failure:
data = nil
handle = <STATUS_TABLE>	-- see description for ibclr()
errmsg = "Error code and detailed description"


-- Example 2: read 12 bytes from device 4 as table of ASCII-characters
--            devData = "ABc" : data[1]="A" data[2]="B" data[3]="c"
local data, stat, errmsg = gpib.ibrd(4,12,"charTable")
-- On success:
data = <TABLE_OF_CHARACTERS>	-- with Lua 1-based indexing
stat = <STATUS_TABLE>	-- see description for ibclr()
errmsg = nil	-- no error message
-- On failure:
data = nil
handle = <STATUS_TABLE>	-- see description for ibclr()
errmsg = "Error code and detailed description"


-- Example 3: read 8 bytes from device 5 as table of numbers (raw data)
--            devData = "ABc" : data[1]=0x41 data[2]=0x42 data[3]=0x63
local data, stat, errmsg = gpib.ibrd(5,8,"binTable")
-- On success:
data = <TABLE_OF_NUMBERS>	-- with Lua 1-based indexing
stat = <STATUS_TABLE>	-- see description for ibclr()
errmsg = nil	-- no error message
-- On failure:
data = nil
handle = <STATUS_TABLE>	-- see description for ibclr()
errmsg = "Error code and detailed description"
```

