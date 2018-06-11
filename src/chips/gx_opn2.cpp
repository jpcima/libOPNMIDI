#include "gx_opn2.h"
#include <cstring>

#include "gx/gx_ym2612.h"

GXOPN2::GXOPN2()
    : m_chip(YM2612GXAlloc()),
      m_framecount(0),
      m_bufindex(0),
      m_buflength(0)
{
    YM2612GX *chip = m_chip;
    YM2612GXInit(chip);
    YM2612GXConfig(chip, YM2612_DISCRETE);
    setRate(m_rate, m_clock);
}

GXOPN2::~GXOPN2()
{
    YM2612GX *chip = m_chip;
    YM2612GXFree(chip);
}

void GXOPN2::setRate(uint32_t rate, uint32_t clock)
{
    YM2612GX *chip = m_chip;
    OPNChipBaseT::setRate(rate, clock);
    YM2612GXResetChip(chip);
}

void GXOPN2::reset()
{
    YM2612GX *chip = m_chip;
    OPNChipBaseT::reset();
    YM2612GXResetChip(chip);
}

void GXOPN2::writeReg(uint32_t port, uint16_t addr, uint8_t data)
{
    YM2612GX *chip = m_chip;

    // run one command if full buffer
    if(m_buflength == BufferMax) {
        BufferedWrite ent;
        ent = m_buf[m_bufindex];
        m_bufindex = (m_bufindex + 1) % BufferMax;
        --m_buflength;
        YM2612GXWrite(chip, 0 + ent.port * 2, ent.addr);
        YM2612GXWrite(chip, 1 + ent.port * 2, ent.data);
    }

    // add to buffer
    BufferedWrite ent = { port, addr, data };
    m_buf[(m_bufindex + m_buflength) % BufferMax] = ent;
    ++m_buflength;
}

void GXOPN2::nativePreGenerate()
{
    YM2612GX *chip = m_chip;
    YM2612GXPreGenerate(chip);
    m_framecount = 0;
}

void GXOPN2::nativePostGenerate()
{
    YM2612GX *chip = m_chip;
    YM2612GXPostGenerate(chip, m_framecount);
}

void GXOPN2::nativeGenerate(int16_t *frame)
{
    YM2612GX *chip = m_chip;
    YM2612GXGenerateOneNative(chip, frame);
    ++m_framecount;

    // run one command from buffer
    if(m_buflength > 0) {
        BufferedWrite ent;
        ent = m_buf[m_bufindex];
        m_bufindex = (m_bufindex + 1) % BufferMax;
        --m_buflength;
        YM2612GXWrite(chip, 0 + ent.port * 2, ent.addr);
        YM2612GXWrite(chip, 1 + ent.port * 2, ent.data);
    }
}

const char *GXOPN2::emulatorName()
{
    return "Genesis Plus GX";
}
