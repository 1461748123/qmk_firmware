#include <stdint.h>
#include "i2c_master.h"

#define MCP23017
#define MCP23017_A0 0
#define MCP23017_A1 0
#define MCP23017_A2 0

#ifdef MCP23017
#define EXPANDER_ADDR ((0x20|(MCP23017_A0<<0)|(MCP23017_A1<<1)|(MCP23017_A2<<2)) << 1)
enum EXPANDER_REG_BANK0 {
  EXPANDER_REG_IODIRA = 0,
  EXPANDER_REG_IODIRB,
  EXPANDER_REG_IPOLA,
  EXPANDER_REG_IPOLB,
  EXPANDER_REG_GPINTENA,
  EXPANDER_REG_GPINTENB,
  EXPANDER_REG_DEFVALA,
  EXPANDER_REG_DEFVALB,
  EXPANDER_REG_INTCONA,
  EXPANDER_REG_INTCONB,
  EXPANDER_REG_IOCONA,
  EXPANDER_REG_IOCONB,
  EXPANDER_REG_GPPUA,
  EXPANDER_REG_GPPUB,
  EXPANDER_REG_INTFA,
  EXPANDER_REG_INTFB,
  EXPANDER_REG_INTCAPA,
  EXPANDER_REG_INTCAPB,
  EXPANDER_REG_GPIOA,
  EXPANDER_REG_GPIOB,
  EXPANDER_REG_OLATA,
  EXPANDER_REG_OLATB
};
#endif

#define EXPANDER_TIMEOUT 1

void expander_init(void);
i2c_status_t expander_write(uint8_t reg, uint8_t data);
