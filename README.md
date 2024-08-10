# lua4882
Access to NI's NI-488.2 (GPIB) driver

## Usage

### ibdev()

Purpose: Open and initialize a GPIB device descriptor.

```lua
local gpib = require "lua4882"
local boardIndex = 0	-- Commonly used index of bus controller
local primaryAddr = 1	-- Primary GPIB address of remote device
local secondaryAddr = 0	-- No secondary GPIB address of remote device
local timeout = 11		-- Timeout as integer from 0 to 17
						--  0: Infinite timeout (disabled)
						--  1: Timeout of 10 us (ideal)
						--  2: Timeout of 30 us (ideal)
						--  3: Timeout of 100 us (ideal)
						--  4: Timeout of 300 us (ideal)
						--  5: Timeout of 1 ms (ideal)
						--  6: Timeout of 3 ms (ideal)
						--  7: Timeout of 10 ms (ideal)
						--  8: Timeout of 30 ms (ideal)
						--  9: Timeout of 100 ms (ideal)
						-- 10: Timeout of 300 ms (ideal)
						-- 11: Timeout of 1 s (ideal)
						-- 12: Timeout of 3 s (ideal)
						-- 13: Timeout of 10 s (ideal)
						-- 14: Timeout of 30 s (ideal)
						-- 15: Timeout of 100 s (ideal)
						-- 16: Timeout of 300 s (ideal)
						-- 17: Timeout of 1000 s (ideal)
local eoiMode = 1		-- Asserts GPIB End-or-Identify (EOI) line at end of transfer
local eosMode = 0		-- No end-of-string character

local handle, errmsg = gpib.ibdev(boardIndex, primaryAddr, secondaryAddr, timeout, eoiMode, eosMode)

-- on success:
handle = <DEVICE_HANDLE>
errmsg = nil
-- on failure:
handle = nil
errmsg = "Error code and detailed description"

-- Example (without an GPIB-card installed)
Lua 5.4.7  Copyright (C) 1994-2024 Lua.org, PUC-Rio
> gpib=require "lua4882"
> gpib.ibdev(0,3,0,3,1,0)
nil     ENEB: Non-existent interface board
```



