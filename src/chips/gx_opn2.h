#ifndef GX_OPN2_H
#define GX_OPN2_H

#include "opn_chip_base.h"

struct YM2612GX;
class GXOPN2 final : public OPNChipBaseT<GXOPN2>
{
    YM2612GX *m_chip;
    unsigned int m_framecount;
public:
    GXOPN2();
    ~GXOPN2() override;

    void setRate(uint32_t rate, uint32_t clock) override;
    void reset() override;
    void writeReg(uint32_t port, uint16_t addr, uint8_t data) override;
    void nativePreGenerate() override;
    void nativePostGenerate() override;
    void nativeGenerate(int16_t *frame) override;
    const char *emulatorName() override;

private:
    enum { WriteCycle = 15 };
    unsigned m_cycle;
    struct BufferedWrite { unsigned port, addr, data; bool ready; };
    enum { BufferMax = 256 };
    unsigned m_bufindex;
    unsigned m_buflength;
    BufferedWrite m_buf[BufferMax];
};

#endif // GX_OPN2_H
