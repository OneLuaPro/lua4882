/*
--------------------------------------------------------------------------------
MIT License

lua4882 - Copyright (c) 2024 Kritzel Kratzel.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

--------------------------------------------------------------------------------

National Instruments NI-488.2 API
Copyright (c) National Instruments 2001-2007. All Rights Reserved.
https://www.ni.com/en/about-ni/legal/software-license-agreement.html

--------------------------------------------------------------------------------
*/

#ifdef _WINDLL
#include <windows.h>
#define DLL __declspec(dllexport)
#else
#define DLL //empty
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <ni4882.h>

#define TRUE 1
#define FALSE 0
#define ASCIISTRING 0
#define CHARTABLE   1
#define BINTABLE    2
#define NUM_OPTIONS_IBCONFIG	26
#define LUA4882_VERSION "lua4882 v1.0"

// Ibconfig() Ibask() options, taken from ni4882.h
const char optMnemonic[NUM_OPTIONS_IBCONFIG][18] = {
  "IbcPAD", "IbcSAD", "IbcTMO", "IbcEOT", "IbcPPC", "IbcREADDR", "IbcAUTOPOLL",
  "IbcSC", "IbcSRE", "IbcEOSrd", "IbcEOSwrt", "IbcEOScmp", "IbcEOSchar",
  "IbcPP2", "IbcTIMING", "IbcDMA", "IbcSendLLO", "IbcSPollTime", "IbcPPollTime",
  "IbcEndBitIsNormal", "IbcUnAddr", "IbcHSCableLength", "IbcIst", "IbcRsv",
  "IbcLON", "IbcEOS"};

const unsigned int optCode[NUM_OPTIONS_IBCONFIG] = {
  IbcPAD, IbcSAD, IbcTMO, IbcEOT, IbcPPC, IbcREADDR, IbcAUTOPOLL,
  IbcSC, IbcSRE, IbcEOSrd, IbcEOSwrt, IbcEOScmp, IbcEOSchar,
  IbcPP2, IbcTIMING, IbcDMA, IbcSendLLO, IbcSPollTime, IbcPPollTime,
  IbcEndBitIsNormal, IbcUnAddr, IbcHSCableLength, IbcIst, IbcRsv,
  IbcLON, IbcEOS};

#ifdef _WINDLL

//------------------------------------------------------------------------------
static const char* errorMnemonic(int err) {
  // Error messages corresponding to global variable iberr
  const char *msg;
  switch(err) {
  case EDVR:
    msg = "EDVR:System error";
    break;
  case ECIC:
    msg = "ECIC:Function requires GPIB board to be CIC";
    break;
  case ENOL:
    msg = "ENOL:Write function detected no Listeners";
    break;
  case EADR:
    msg = "EADR:Interface board not addressed correctly";
    break;
  case EARG:
    msg = "EARG:Invalid argument to function call";
    break;
  case ESAC:
    msg = "ESAC:Function requires GPIB board to be SAC";
    break;
  case EABO:
    msg = "EABO:I/O operation aborted";
    break;
  case ENEB:
    msg = "ENEB:Non-existent interface board";
    break;
  case EDMA:
    msg = "EDMA:Error performing DMA";
    break;
  case EOIP:
    msg = "EOIP:I/O operation started before previous operation completed";
    break;
  case ECAP:
    msg = "ECAP:No capability for intended operation";
    break;
  case EFSO:
    msg = "EFSO:File system operation error";
    break;
  case EBUS:
    msg = "EBUS:Command error during device call";
    break;
  case ESTB:
    msg = "ESTB:Serial poll status byte lost";
    break;
  case ESRQ:
    msg = "ESRQ:SRQ remains asserted";
    break;
  case ETAB:
    msg = "ETAB:The return buffer is full";
    break;
  case ELCK:
    msg = "ELCK:Address or board is locked";
    break;
  case EARM:
    msg = "EARM:The ibnotify Callback failed to rearm";
    break;
  case EHDL:
    msg = "EHDL:The input handle is invalid";
    break;
  case WCFG:
    msg = "WCFG:Configuration warning";
    break;
  case EWIP:
    msg = "EWIP:Wait already in progress on input ud";
    break;
  case ERST:
    msg = "ERST:The event notification was cancelled due to a reset of the interface";
    break;
  case EPWR:
    msg = "EPWR:The system or board has lost power or gone to standby";
    break;
  default:
    msg = "E???:Undocumented error code";
    break;
  }
  return msg;
}

//------------------------------------------------------------------------------
static void pushIbsta(lua_State *L, unsigned int status){
  // Pushes content of Ibsta on stack (as table) as return value
  
  const char bitMnemonic[16][5] = {"DCAS","DTAS","LACS","TACS",
				   "ATN", "CIC", "REM", "LOK",
				   "CMPL","",    "",    "RQS",
				   "SRQI","END", "TIMO","ERR"};
  // build table
  lua_newtable(L);
  int top = lua_gettop(L);

  // push index/value-pairs on stack
  for (int i=0; i<16; i++) {
    // skip unused bits
    if (strcmp(bitMnemonic[i],"") == 0) continue;
    // push index
    lua_pushstring(L,bitMnemonic[i]);
    // push value
    lua_pushboolean(L,(status & (1 << i)) >> i);
    // assign table[index] = value
    lua_settable(L,top); // also pops processed index/value from stack
  }
}

//------------------------------------------------------------------------------
static int luaL_checkboolean(lua_State *L, int idx){
  // There is no luaL_checkboolean() in Lua. This is my lousy implementation.
  // https://www.lua.org/manual/5.4/manual.html#lua_toboolean
  if (lua_isboolean(L,idx)) {
    return lua_toboolean(L,idx);	// returns 0 or 1
  }
  else {
    return luaL_error(L,"bad argument in caller of luaL_checkboolean (boolean expected)");
  }
}

//------------------------------------------------------------------------------
static int lua4882_ibask(lua_State *L) {
  // Return information about software configuration parameters.
  // unsigned int ibask (int ud, int option, int *value)

  // Check number of arguments
  if (lua_gettop(L) != 2) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  const char *givenOptName = luaL_checkstring(L,2);
  int givenOptIdx = -1;	// init to impossible value
  for (int i=0; i<NUM_OPTIONS_IBCONFIG; i++) {
    if (strcmp(optMnemonic[i],givenOptName) == 0) {
      // given option name is valid
      givenOptIdx = i;
      break;	// done here
    }
  }
  if (givenOptIdx == -1) {
    // Given option not found.
    return luaL_error(L,"Unknown Ibask() option name.");
  }
  int optVal;
  // Call C-function
  unsigned int status = ibask(descr,optCode[givenOptIdx],&optVal);
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    lua_pushnil(L);				// nothing to return
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    lua_pushinteger(L,optVal);			// Current option content
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 3;
}

//------------------------------------------------------------------------------
static int lua4882_ibclr(lua_State *L){
  // Clear a specific device.
  // unsigned int ibclr (int ud)

  // Check number of arguments
  if (lua_gettop(L) != 1) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  // Call C-function
  unsigned int status = ibclr(descr);
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 2;
}

//------------------------------------------------------------------------------
static int lua4882_ibconfig(lua_State *L) {
  // Change the software configuration input.
  // unsigned int ibconfig (int ud, int option, int value)

  // Check number of arguments
  if (lua_gettop(L) != 3) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  const char *givenOptName = luaL_checkstring(L,2);
  int givenOptIdx = -1;	// init to impossible value
  for (int i=0; i<NUM_OPTIONS_IBCONFIG; i++) {
    if (strcmp(optMnemonic[i],givenOptName) == 0) {
      // given option name is valid
      givenOptIdx = i;
      break;	// done here
    }
  }
  if (givenOptIdx == -1) {
    // Given option not found.
    return luaL_error(L,"Unknown Ibconfig() option name.");
  }
  int optArg = luaL_checkinteger(L,3);
  // Call C-function
  unsigned int status = ibconfig(descr,optCode[givenOptIdx],optArg);
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 2;
}

//------------------------------------------------------------------------------
static int lua4882_ibdev(lua_State *L) {
  // Open and initialize a device descriptor.
  // int ibdev (int BdIndx, int pad, int sad, int tmo, int eot, int eos)

  // Check number of arguments
  if (lua_gettop(L) != 6) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  int pad   = (int)luaL_checkinteger(L,2);
  int sad   = (int)luaL_checkinteger(L,3);
  int tmo   = (int)luaL_checkinteger(L,4);
  int eot   = (int)luaL_checkinteger(L,5);
  int eos   = (int)luaL_checkinteger(L,6);
  // Call C-function
  int handle = ibdev(descr, pad, sad, tmo, eot, eos);
  // Error handling
  if (Ibsta() & ERR) {
    // failed
    lua_pushnil(L);		// no handle or descriptor
    lua_pushstring(L,errorMnemonic(Iberr())); // errmsg
  }
  else {
    // OK
    lua_pushinteger(L,handle);	// device descriptor
    lua_pushnil(L);		// no errmsg
  }
  return 2;
}

//------------------------------------------------------------------------------
static int lua4882_ibonl(lua_State *L) {
  // Place the device or interface online or offline.
  // unsigned int ibonl (int ud, int v)

  // Check number of arguments
  if (lua_gettop(L) != 2) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  int state = luaL_checkboolean(L,2);
  // Call C-function
  unsigned int status = ibonl(descr,state);
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 2;
}
  
//------------------------------------------------------------------------------
static int lua4882_ibrd(lua_State *L) {
  // Read data from a device into a user buffer. Operation terminates normally
  // when count bytes have been received or END is received.
  // unsigned int ibrd (int ud, void *rdbuf, size_t count)
  //
  // Additional option for configuring read data to Lua:
  // No option   : Return data as normal ASCII-string
  // "charTable" : Return data as separate ASCII-chars in table with 1-based
  //               indexing. str="ABc" -> t[1]="A" t[2]="B" t[3]="c"
  // "binTable"  : Return data as separate binary numbers in table with 1-based
  //               indexing. str="ABc" -> t[1]=0x41 t[2]=0x42 t[3]=0x63

  // Check arguments
  int descr, output;
  size_t count;
  const char *txtOption = '\0';
  if (lua_gettop(L) == 2) {
    // Two arguments, output as normal ASCII-string.
    descr = (int)luaL_checkinteger(L,1);
    count = (size_t)luaL_checkinteger(L,2);
    output = ASCIISTRING;
  }
  else if (lua_gettop(L) == 3) {
    // Three arguments, output dependent on given option
    descr = (int)luaL_checkinteger(L,1);
    count = (size_t)luaL_checkinteger(L,2);
    txtOption = luaL_checkstring(L,3);
    if (strcmp(txtOption,"charTable") == 0) {
      output = CHARTABLE;
    }
    else if (strcmp(txtOption,"binTable") == 0) {
      output = BINTABLE;
    }
    else {
      return luaL_error(L,"Optional 3rd argument must be either \"charTable\" or \"binTable\".");
    }
  } else {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Preparations
  char *rdBuf = malloc(count*sizeof(char));
  // Call C-function
  unsigned int status = ibrd(descr, rdBuf, count);	// Actual number of bytes
  						     	// transferred is returned
  							// in Ibcnt(), see below.
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    lua_pushnil(L);				// no received data
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    if (output == CHARTABLE || output == BINTABLE) {
      // output data as table
      lua_newtable(L);
      int top = lua_gettop(L);
      // push index/value-pairs on stack
      for (int i = 0; i < min(Ibcnt(),count); i++) {
	// push index
	lua_pushinteger(L,i+1); 		// Lua 1-based indexing
	// push value...
	if (output == BINTABLE) {
	  // ...as binary data table element, make sure that just the byte
	  // is pushed by masking with 0xff.
	  lua_pushinteger(L,((lua_Integer)rdBuf[i]) & 0xff);
	}
	else {
	  // ...as ASCII data table element
	  lua_pushlstring(L,&(rdBuf[i]),1);
	}
	// assign table[index] = value
	lua_settable(L,top); // also pops processed index/value from stack
      }
    }
    else {
      // output data as string
      lua_pushlstring(L,rdBuf,min(Ibcnt(),count));// push data as string
    }
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  // Cleanup
  free(rdBuf);
  return 3;
}

//------------------------------------------------------------------------------
static int lua4882_ibrsp(lua_State *L){
  // Conduct a serial poll.
  // unsigned int ibrsp (int ud, char *spr)

  const char bitMnemonic[8][5] ={"bit0","bit1","bit2","bit3","bit4","bit5",
    "bit6","bit7"};
  
  // Check number of arguments
  if (lua_gettop(L) != 1) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  // Call C-function
  char response = 0x0;
  unsigned int status = ibrsp(descr,&response);
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    lua_pushnil(L);				// no response byte
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    // build response byte table
    lua_newtable(L);
    int top = lua_gettop(L);
    // push index/value-pairs on stack
    for (int i=0; i<8; i++) {
      // push index
      lua_pushstring(L,bitMnemonic[i]);
      // push value
      lua_pushboolean(L,(response & (1 << i)) >> i);
      // assign table[index] = value
      lua_settable(L,top); // also pops processed index/value from stack
    }
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 3;
}

//------------------------------------------------------------------------------
static int lua4882_ibtrg(lua_State *L){
  // Trigger selected device.
  // unsigned int ibtrg (int ud)

  // Check number of arguments
  if (lua_gettop(L) != 1) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  // Call C-function
  unsigned int status = ibtrg(descr);
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 2;
}

//------------------------------------------------------------------------------
static int lua4882_ibwait(lua_State *L) {
  // Wait for GPIB events.
  // unsigned int ibwait (int ud, int mask)

  int waitMaskValue = 0x0;
  const char maskMnemonic[15][5] = {"DCAS","DTAS","LACS","TACS","ATN","CIC",
    "REM","LOK","CMPL","","","RQS","SRQI","END","TIMO"};
  
  // Check number of arguments
  if (lua_gettop(L) != 2) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  if (lua_isstring(L,2)) {
    // Just one single wait mask
    const char *waitMaskName = luaL_checkstring(L,2);
    int bitIdx = -1;	// init to impossible value
    for (int i=0; i<15; i++) {
      // skip unused bit positions
      if (strcmp(maskMnemonic[i],"") == 0) continue;
      // check current wait mask name
      if (strcmp(maskMnemonic[i],waitMaskName) == 0) {
	// given wait mask is valid
	bitIdx = i;
	break;	// done here
      }
    }
    if (bitIdx == -1) {
      // Wait mask name not found.
      return luaL_argerror(L,2,"Unknown wait mask name.");
    }
    // Generate actual wait mask
    waitMaskValue = (1 << bitIdx);
  }
  else if (lua_istable(L,2)) {
    // A table of several wait masks
    size_t len = lua_rawlen(L, 2);
    if (len == 0) {
      return luaL_error(L,"2nd argument is a table of length 0.");
    }
    // Traverse table (https://www.lua.org/manual/5.4/manual.html#lua_next)
    lua_pushnil(L);
    while(lua_next(L,2) != 0) {
      // Key at index -2 (useless here)
      int key = (int)luaL_checkinteger(L,-2);
      // Value (the wait mask) at index -1
      const char *waitMaskName = luaL_checkstring(L,-1);
      // Pop useless key for next cycle
      lua_pop(L,1);
      // Check validity of current wait mask
      int bitIdx = -1;	// init to impossible value
      for (int i=0; i<15; i++) {
	// skip unused bit positions
	if (strcmp(maskMnemonic[i],"") == 0) continue;
	// check current wait mask name
	if (strcmp(maskMnemonic[i],waitMaskName) == 0) {
	  // given wait mask is valid
	  bitIdx = i;
	  break;	// done here
	}
      }
      // Abort if necessary
      if (bitIdx == -1) {
	// Wait mask name not found.
	return luaL_argerror(L,2,"Unknown wait mask name in table.");
      }
      // Apply current wait mask to waitMaskValue
      waitMaskValue |= (1 << bitIdx);
    }
  }
  else {
    return luaL_argerror(L,2,"Argument must be either a string or a table.");
  }
  // Call C-function
  // printf("waitMaskValue = 0x%04x\n",waitMaskValue);
  unsigned int status = ibwait(descr,waitMaskValue);
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 2;
}

//------------------------------------------------------------------------------
static int lua4882_ibwrt(lua_State *L) {
  // Write data to a device from a user buffer.
  // unsigned int ibwrt (int ud, const void *wrtbuf, size_t count)

  // Check number of arguments
  if (lua_gettop(L) != 2) {
    // bailing out
    return luaL_error(L,"Wrong number of arguments.");
  }
  // Check arguments
  int descr = (int)luaL_checkinteger(L,1);
  const char *txData = luaL_checkstring(L,2);
  // Call C-function
  unsigned int status = ibwrt(descr,txData,strlen(txData));
  // Result and error handling
  if (Ibsta() & ERR) {
    // failed
    lua_pushnil(L);				// no number of bytes sent
    pushIbsta(L,status);			// IBSTA table
    lua_pushstring(L,errorMnemonic(Iberr()));	// errmsg
  }
  else {
    // OK
    lua_pushinteger(L,(lua_Integer)Ibcnt());	// Number of bytes sent
    pushIbsta(L,status);			// IBSTA table
    lua_pushnil(L);				// no errmsg
  }
  return 3;
}

#else
// FIXME - non-_WINDLL not yet implemented
#endif

//------------------------------------------------------------------------------
static const struct luaL_Reg lua4882_metamethods [] = {
  {"__call", lua4882_ibask},
  {"__call", lua4882_ibclr},
  {"__call", lua4882_ibconfig},
  {"__call", lua4882_ibdev},
  {"__call", lua4882_ibonl},
  {"__call", lua4882_ibrd},
  {"__call", lua4882_ibrsp},
  {"__call", lua4882_ibtrg},
  {"__call", lua4882_ibwait},
  {"__call", lua4882_ibwrt},
  {NULL, NULL}
};

static const struct luaL_Reg lua4882_funcs [] = {
  {"ibask",    lua4882_ibask},
  {"ibclr",    lua4882_ibclr},
  {"ibconfig", lua4882_ibconfig},
  {"ibdev",    lua4882_ibdev},
  {"ibonl",    lua4882_ibonl},
  {"ibrd",     lua4882_ibrd},
  {"ibrsp",    lua4882_ibrsp},
  {"ibtrg",    lua4882_ibtrg},
  {"ibwait",   lua4882_ibwait},
  {"ibwrt",    lua4882_ibwrt},
  {NULL, NULL}
};

DLL int luaopen_lua4882(lua_State *L){
  luaL_newlib(L, lua4882_funcs);
  luaL_newlib(L, lua4882_metamethods);
  lua_setmetatable(L, -2);
  lua_pushliteral(L,LUA4882_VERSION);
  lua_setfield(L,-2,"_VERSION");
  return 1;
}
//------------------------------------------------------------------------------
