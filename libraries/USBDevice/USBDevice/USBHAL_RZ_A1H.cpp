/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if defined(TARGET_RZ_A1H)

/*
  This class can use the pipe1, pipe3 and pipe6 only. You should
  re-program this class if you wanted to use other pipe.
 */

/*************************************************************************/
extern "C"
{
#include "r_typedefs.h"
#include "iodefine.h"
}
#include "USBHAL.h"
//#include "USBRegs_RZ_A1H.h"


/*for debug print*/
//#define  DEBUG_RZ_A1H


/*************************************************************************/
const struct PIPECFGREC {
    uint16_t    endpoint;
    uint16_t    pipesel;
    uint16_t    pipecfg;
    uint16_t    pipebuf;
    uint16_t    pipemaxp;
    uint16_t    pipeperi;
} def_pipecfg[] = {
    /*EP0OUT and EP0IN are configured by USB IP*/
    {
        EP1OUT, /*EP1: Host -> Func, INT*/
        6 | USB_FUNCTION_D0FIFO_USE,
        USB_FUNCTION_INTERRUPT |
        USB_FUNCTION_BFREOFF   |
        USB_FUNCTION_DBLBOFF   |
        USB_FUNCTION_CNTMDON   |
        USB_FUNCTION_SHTNAKOFF |
        USB_FUNCTION_DIR_P_OUT |
        USB_FUNCTION_EP1,
        ( ( (  64) / 64 - 1 ) << 10 ) | 0x04u,
        64,
        DEVDRV_USBF_OFF | 0,
    },
    {
        EP1IN,  /*EP1: Host <- Func, INT*/
        7 | USB_FUNCTION_D1FIFO_USE,
        USB_FUNCTION_INTERRUPT |
        USB_FUNCTION_BFREOFF   |
        USB_FUNCTION_DBLBOFF   |
        USB_FUNCTION_CNTMDOFF  |
        USB_FUNCTION_SHTNAKOFF |
        USB_FUNCTION_DIR_P_IN  |
        USB_FUNCTION_EP1,
        ( ( (  64) / 64 - 1 ) << 10 ) | 0x05u,
        64,
        DEVDRV_USBF_OFF | 0,
    },
    {
        EP2OUT, /*EP2: Host -> Func, BULK*/
        3 | USB_FUNCTION_D0FIFO_USE,
        USB_FUNCTION_BULK      |
        USB_FUNCTION_BFREOFF   |
        USB_FUNCTION_DBLBON    |
        USB_FUNCTION_CNTMDON   |
        USB_FUNCTION_SHTNAKON  |
        USB_FUNCTION_DIR_P_OUT |
        USB_FUNCTION_EP2,
        ( ( (2048) / 64 - 1 ) << 10 ) | 0x30u,
        512,
        DEVDRV_USBF_OFF | 0,
    },
    {
        EP2IN,  /*EP2: Host <- Func, BULK*/
        4 | USB_FUNCTION_D1FIFO_USE,
        USB_FUNCTION_BULK      |
        USB_FUNCTION_BFREOFF   |
        USB_FUNCTION_DBLBOFF   |
        USB_FUNCTION_CNTMDON   |
        USB_FUNCTION_SHTNAKOFF |
        USB_FUNCTION_DIR_P_IN  |
        USB_FUNCTION_EP2,
        ( ( (2048) / 64 - 1 ) << 10 ) | 0x50u,
        512,
        DEVDRV_USBF_OFF | 0,
    },
    {
        EP3OUT, /*EP3: Host -> Func, ISO*/
        1 | USB_FUNCTION_D0FIFO_USE,
        USB_FUNCTION_ISO       |
        USB_FUNCTION_BFREOFF   |
        USB_FUNCTION_DBLBON    |
        USB_FUNCTION_CNTMDON   |
        USB_FUNCTION_SHTNAKON  |
        USB_FUNCTION_DIR_P_OUT |
        USB_FUNCTION_EP3,
        ( ( (1024) / 64 - 1 ) << 10 ) | 0x10u,
        192,
        DEVDRV_USBF_OFF | 1,
    },
    {
        EP3IN,  /*EP3: Host <- Func, ISO*/
        2 | USB_FUNCTION_D1FIFO_USE,
        USB_FUNCTION_ISO       |
        USB_FUNCTION_BFREOFF   |
        USB_FUNCTION_DBLBON    |
        USB_FUNCTION_CNTMDON   |
        USB_FUNCTION_SHTNAKOFF |
        USB_FUNCTION_DIR_P_IN  |
        USB_FUNCTION_EP3,
        ( ( (1024) / 64 - 1 ) << 10 ) | 0x20u,
        192,
        DEVDRV_USBF_OFF | 1,
    },
    { /*terminator*/
        0, 0, 0, 0, 0,
    },
};


/*************************************************************************/
USBHAL * USBHAL::instance;


static uint16_t setup_buffer[MAX_PACKET_SIZE_EP0 / 2];

/* 0: not used / other: a pipe number to use recv_buffer*/
static uint8_t  recv_buffer[MAX_PACKET_SIZE_EPBULK];
volatile static uint16_t    recv_error;


/*************************************************************************/
extern "C"
{
    void usb0_function_interrupt(uint32_t int_sense)
    {
        USBHAL::instance->_usbisr();
    }
}


/*************************************************************************/
/*EP2PIPE converter is for pipe1, pipe3 and pipe6 only.*/
uint32_t
USBHAL::EP2PIPE(uint8_t endpoint)
{
    return (uint32_t)usb0_function_EpToPipe(endpoint);
}


/*************************************************************************/
#if 0   // No implements
uint32_t USBHAL::endpointReadcore(uint8_t endpoint, uint8_t *buffer)
{
    return 0;
}
#endif

/*************************************************************************/
/* constructor */
USBHAL::USBHAL(void)
{
    /* ---- P4_1 : P4_1 (USB0_EN for GR-PEACH) ---- */
    usb0_en = new DigitalOut(P4_1, 1);

#if 0   /* This setting is same as a routine in  usb0_api_function_init */
    CPG.STBCR7 &= ~(1u<<0u);
    volatile uint8_t    dummy8;
    dummy8 = CPG.STBCR7;
#endif

    /* some constants */
    int_id          = USBI0_IRQn;
    int_level       = ( 2 << 3 );
    clock_mode      = USBFCLOCK_X1_48MHZ;
    mode            = USB_FUNCTION_HIGH_SPEED;
    EP0_read_status = DEVDRV_USBF_WRITEEND;
    EPx_read_status = DEVDRV_USBF_PIPE_DONE;

    /* Disables interrupt for usb */
    GIC_DisableIRQ(int_id);

    /* Setup the end point */
    epCallback[ 0] = &USBHAL::EP1_OUT_callback;
    epCallback[ 1] = &USBHAL::EP1_IN_callback;
    epCallback[ 2] = &USBHAL::EP2_OUT_callback;
    epCallback[ 3] = &USBHAL::EP2_IN_callback;
    epCallback[ 4] = &USBHAL::EP3_OUT_callback;
    epCallback[ 5] = &USBHAL::EP3_IN_callback;
    epCallback[ 6] = &USBHAL::EP4_OUT_callback;
    epCallback[ 7] = &USBHAL::EP4_IN_callback;
    epCallback[ 8] = &USBHAL::EP5_OUT_callback;
    epCallback[ 9] = &USBHAL::EP5_IN_callback;
    epCallback[10] = &USBHAL::EP6_OUT_callback;
    epCallback[11] = &USBHAL::EP6_IN_callback;
    epCallback[12] = &USBHAL::EP7_OUT_callback;
    epCallback[13] = &USBHAL::EP7_IN_callback;
    epCallback[14] = &USBHAL::EP8_OUT_callback;
    epCallback[15] = &USBHAL::EP8_IN_callback;
    epCallback[16] = &USBHAL::EP9_OUT_callback;
    epCallback[17] = &USBHAL::EP9_IN_callback;
    epCallback[18] = &USBHAL::EP10_OUT_callback;
    epCallback[19] = &USBHAL::EP10_IN_callback;
    epCallback[20] = &USBHAL::EP11_OUT_callback;
    epCallback[21] = &USBHAL::EP11_IN_callback;
    epCallback[22] = &USBHAL::EP12_OUT_callback;
    epCallback[23] = &USBHAL::EP12_IN_callback;
    epCallback[24] = &USBHAL::EP13_OUT_callback;
    epCallback[25] = &USBHAL::EP13_IN_callback;
    epCallback[26] = &USBHAL::EP14_OUT_callback;
    epCallback[27] = &USBHAL::EP14_IN_callback;
    epCallback[28] = &USBHAL::EP15_OUT_callback;
    epCallback[29] = &USBHAL::EP15_IN_callback;

    /* registers me */
    instance = this;

    /* Clear pipe table */
    usb0_function_clear_pipe_tbl();

    /* Initialize USB IP */
    usb0_api_function_init(int_level, mode, clock_mode);

    {
        uint16_t buf;
        buf  = USB200.INTENB0;
        buf |= USB_INTENB0_SOFE;
        USB200.INTENB0 = buf;
    }
}

/*************************************************************************/
USBHAL::~USBHAL(void)
{
    /* Disables interrupt for usb */
    GIC_DisableIRQ( int_id );
    /* Unregisters interrupt function and priority */
    InterruptHandlerRegister( int_id, (uint32_t)NULL );

    usb0_en  = NULL;
    instance = NULL;
}

/*************************************************************************/
void USBHAL::connect(void)
{
    /* Activates USB0_EN */
    (*usb0_en) = 0;
}


/*************************************************************************/
void USBHAL::disconnect(void)
{
    /* Deactivates USB0_EN */
    (*usb0_en) = 1;
}


/*************************************************************************/
void USBHAL::configureDevice(void)
{
    /*The pipes set up in USBHAL::realiseEndpoint*/
    /*usb0_function_clear_alt();*/      /* Alternate setting clear */
    /*usb0_function_set_pid_buf(USB_FUNCTION_PIPE0);*/
}


/*************************************************************************/
void USBHAL::unconfigureDevice(void)
{
    /* The Interface would be managed by USBDevice */
    /*usb0_function_clear_alt();*/      /* Alternate setting clear */
    /*usb0_function_set_pid_buf(USB_FUNCTION_PIPE0);*/
}


/*************************************************************************/
void USBHAL::setAddress(uint8_t address)
{
    if (address <= 127) {
        usb0_function_set_pid_buf(USB_FUNCTION_PIPE0);      /* OK */
    } else {
        usb0_function_set_pid_stall(USB_FUNCTION_PIPE0);    /* Not Spec */
    }
}


/*************************************************************************/
bool USBHAL::realiseEndpoint(uint8_t endpoint, uint32_t maxPacket, uint32_t flags)
{
    const struct PIPECFGREC *cfg;
    uint16_t pipe;
    uint16_t buf;

    if ( (EP0OUT == endpoint) || (EP0IN  == endpoint) ) {
        return true;
    }

    for (cfg = &def_pipecfg[0]; cfg->pipesel != 0; cfg++) {
        if (cfg->endpoint == endpoint) {
            break;
        }
    }
    if (cfg->pipesel == 0) {
        return false;
    }

    pipe = ((cfg->pipesel & USB_PIPESEL_PIPESEL) >> USB_PIPESEL_PIPESEL_SHIFT);

    g_usb0_function_PipeTbl[ pipe ] = (uint16_t)(endpoint | ((cfg->pipesel & USB_FUNCTION_FIFO_USE) << 0));

    /* There are maintenance routine of SHTNAK and BFRE bits
     * in original sample program. This sample is not
     * programmed. Do maintenance the "def_pipecfg" array if
     * you want it. */

    /* Interrupt Disable */
    buf  = USB200.BRDYENB;
    buf &= (uint16_t)~g_usb0_function_bit_set[pipe];
    USB200.BRDYENB = buf;
    buf  = USB200.NRDYENB;
    buf &= (uint16_t)~g_usb0_function_bit_set[pipe];
    USB200.NRDYENB = buf;
    buf  = USB200.BEMPENB;
    buf &= (uint16_t)~g_usb0_function_bit_set[pipe];
    USB200.BEMPENB = buf;

    usb0_function_set_pid_nak(pipe);

    /* CurrentPIPE Clear */
    if (RZA_IO_RegRead_16(&USB200.CFIFOSEL, USB_CFIFOSEL_CURPIPE_SHIFT, USB_CFIFOSEL_CURPIPE) == pipe) {
        RZA_IO_RegWrite_16(&USB200.CFIFOSEL, 0, USB_CFIFOSEL_CURPIPE_SHIFT, USB_CFIFOSEL_CURPIPE);
    }

    if (RZA_IO_RegRead_16(&USB200.D0FIFOSEL, USB_DnFIFOSEL_CURPIPE_SHIFT, USB_DnFIFOSEL_CURPIPE) == pipe) {
        RZA_IO_RegWrite_16(&USB200.D0FIFOSEL, 0, USB_DnFIFOSEL_CURPIPE_SHIFT, USB_DnFIFOSEL_CURPIPE);
    }

    if (RZA_IO_RegRead_16(&USB200.D1FIFOSEL, USB_DnFIFOSEL_CURPIPE_SHIFT, USB_DnFIFOSEL_CURPIPE) == pipe) {
        RZA_IO_RegWrite_16(&USB200.D1FIFOSEL, 0, USB_DnFIFOSEL_CURPIPE_SHIFT, USB_DnFIFOSEL_CURPIPE);
    }

    /* PIPE Configuration */
    USB200.PIPESEL  = pipe;
    USB200.PIPECFG  = cfg->pipecfg;
    USB200.PIPEBUF  = cfg->pipebuf;
    USB200.PIPEMAXP = cfg->pipemaxp;
    USB200.PIPEPERI = cfg->pipeperi;

    g_usb0_function_pipecfg[pipe]  = cfg->pipecfg;
    g_usb0_function_pipebuf[pipe]  = cfg->pipebuf;
    g_usb0_function_pipemaxp[pipe] = cfg->pipemaxp;
    g_usb0_function_pipeperi[pipe] = cfg->pipeperi;

    /* Buffer Clear */
    usb0_function_set_sqclr(pipe);
    usb0_function_aclrm(pipe);

    /* init Global */
    g_usb0_function_pipe_status[pipe]  = DEVDRV_USBF_PIPE_IDLE;
    g_usb0_function_PipeDataSize[pipe] = 0;

    return true;
}


/*************************************************************************/
// read setup packet
void USBHAL::EP0setup(uint8_t *buffer)
{
    memcpy(buffer, setup_buffer, MAX_PACKET_SIZE_EP0);
}


/*************************************************************************/
void USBHAL::EP0readStage(void)
{
    // No implements
}


/*************************************************************************/
void USBHAL::EP0read(void)
{
    uint8_t *buffer;
    uint32_t size;

    /* remain of last writing */
    while (EP0_read_status != DEVDRV_USBF_WRITEEND) {
        static uint8_t bbb[2] = { 255, 255 };
        EP0write(&bbb[0], 0);
    }

    buffer = (uint8_t*)(&setup_buffer[4]);
    size   = (MAX_PACKET_SIZE_EP0 / 2) - 8;
    usb0_api_function_CtrlWriteStart(size, buffer);
}


/*************************************************************************/
uint32_t USBHAL::EP0getReadResult(uint8_t *buffer)
{
    memcpy(buffer, (uint8_t*)(&setup_buffer[4]), g_usb0_function_PipeDataSize[USB_FUNCTION_PIPE0]);

    return g_usb0_function_PipeDataSize[USB_FUNCTION_PIPE0];
}


/*************************************************************************/
void USBHAL::EP0write(uint8_t *buffer, uint32_t size)
{
    /* zero byte writing */
    if ( (size == 0) && (EP0_read_status == DEVDRV_USBF_WRITEEND) ) {
        return;
    }

    if (EP0_read_status == DEVDRV_USBF_WRITEEND) {
        /*1st block*/
        EP0_read_status = usb0_api_function_CtrlReadStart(size, buffer);
    } else {
        /* waits the last transmission */
        /*other blocks*/
        g_usb0_function_data_count[ USB_FUNCTION_PIPE0 ]    = size;
        g_usb0_function_data_pointer [ USB_FUNCTION_PIPE0 ] = buffer;
        EP0_read_status = usb0_function_write_buffer_c(USB_FUNCTION_PIPE0);
    }
    /*max size may be deblocking outside*/
    if (size == MAX_PACKET_SIZE_EP0) {
        EP0_read_status = DEVDRV_USBF_WRITING;
    }

#if defined(DEBUG_RZ_A1H)
    {
        static const int8_t *statmesg[] = {
            "END",
            "SHRT",
            "ING",
            "DMA",
        };

        printf(
            "call: EP0write(%.4Xh,%d) %s (%d)\n",
            (buffer[0] << 8) | buffer[1],
            size,
            statmesg[ EP0_read_status ],
            g_usb0_function_CtrZeroLengthFlag );
    }
#endif
}


/*************************************************************************/
#if 0   // No implements
void USBHAL::EP0getWriteResult(void)
{
}
#endif

/*************************************************************************/
void USBHAL::EP0stall(void)
{
    stallEndpoint( 0 );
}


/*************************************************************************/
EP_STATUS USBHAL::endpointRead(uint8_t endpoint, uint32_t max_size)
{
    uint32_t    pipe = EP2PIPE(endpoint);
    uint32_t    pipe_size;
    uint16_t    pipe_status;
    EP_STATUS status = EP_COMPLETED;

    pipe_status = usb0_api_function_check_pipe_status(pipe, &pipe_size);
    pipe_size = (max_size < pipe_size)? (max_size): (pipe_size);

    if (pipe_status == DEVDRV_USBF_PIPE_IDLE) {
        usb0_api_function_set_pid_nak(pipe);
        usb0_api_function_clear_pipe_status(pipe);

        usb0_api_function_start_receive_transfer(pipe, pipe_size, recv_buffer);
    } else {
        status = EP_PENDING;
    }

#if defined(DEBUG_RZ_A1H)
    printf( "call: endpointRead(%d,%d) pipe=%d status=%d\n",
            endpoint, max_size, pipe, pipe_status );
#endif

    return status;
}


/*************************************************************************/
EP_STATUS USBHAL::endpointReadResult(uint8_t endpoint, uint8_t *buffer, uint32_t *bytes_read )
{
    uint32_t pipe = EP2PIPE(endpoint);
    uint16_t pipe_status;
    uint16_t err;
    EP_STATUS status = EP_PENDING;


    if (EPx_read_status != DEVDRV_USBF_PIPE_WAIT) {
        return status;
    }

    pipe_status = usb0_api_function_check_pipe_status(pipe, bytes_read);
    if (pipe_status == DEVDRV_USBF_PIPE_IDLE) {
        return EP_COMPLETED;
    }
    if (pipe_status == DEVDRV_USBF_PIPE_DONE) {
        return EP_COMPLETED;
    }
    if (pipe_status != DEVDRV_USBF_PIPE_WAIT) {
        return status;
    }

    /* sets the output buffer and size */
    g_usb0_function_data_pointer[pipe] = buffer;

    /* receives data from pipe */
    err = usb0_function_read_buffer(pipe);
    recv_error = err;

    pipe_status = usb0_api_function_check_pipe_status(pipe, bytes_read);
    if (pipe_status == DEVDRV_USBF_PIPE_DONE) {
        status = EP_COMPLETED;
    }


#if defined(DEBUG_RZ_A1H)
    printf(
        "call: endpointReadResult(%d,%p,%u)=%d pipe=%d status=%d\n",
        endpoint, buffer, *bytes_read, err, pipe, pipe_status );
#endif

    return status;
}


/*************************************************************************/
EP_STATUS USBHAL::endpointWrite(uint8_t endpoint, uint8_t *data, uint32_t size)
{
    uint32_t pipe = EP2PIPE(endpoint);
    uint32_t pipe_size;
    uint16_t pipe_status;
    uint16_t err;
    uint16_t count;
    EP_STATUS status = EP_PENDING;

    pipe_status = usb0_api_function_check_pipe_status(pipe, &pipe_size);

    /* waits the last transmission */
    count = 30000;
    while ((pipe_status == DEVDRV_USBF_PIPE_WAIT) || (pipe_status == DEVDRV_USBF_PIPE_DONE)) {
        pipe_status = usb0_api_function_check_pipe_status(pipe, &pipe_size);
        if( --count == 0 ) {
            pipe_status = DEVDRV_USBF_PIPE_STALL;
            break;
        }
    }

    switch (pipe_status) {
        case DEVDRV_USBF_PIPE_IDLE:
            err = usb0_api_function_start_send_transfer(pipe, size, data);

            switch (err) {
                    /* finish to write */
                case DEVDRV_USBF_WRITEEND:
                    /* finish to write, but data is short */
                case DEVDRV_USBF_WRITESHRT:
                    /* continue to write */
                case DEVDRV_USBF_WRITING:
                    /* use DMA */
                case DEVDRV_USBF_WRITEDMA:
                    /* error */
                case DEVDRV_USBF_FIFOERROR:
                    status = EP_PENDING;
                    break;
            }
            break;

        case DEVDRV_USBF_PIPE_WAIT:
        case DEVDRV_USBF_PIPE_DONE:
            status = EP_PENDING;
            break;

        case DEVDRV_USBF_PIPE_NORES:
        case DEVDRV_USBF_PIPE_STALL:
        default:
            status = EP_STALLED;
            break;
    }

#if defined(DEBUG_RZ_A1H)
    printf(
        "call: endpointWrite(%d,%p,%d)=%d pipe=%d status=%d\n",
        endpoint, data, size, err, pipe, pipe_status );
#endif

    return status;
}


/*************************************************************************/
EP_STATUS USBHAL::endpointWriteResult(uint8_t endpoint)
{
    uint32_t    pipe = EP2PIPE(endpoint);
    uint32_t    pipe_size;
    uint16_t    pipe_status;
    EP_STATUS status = EP_PENDING;

    pipe_status = usb0_api_function_check_pipe_status(pipe, &pipe_size);

    switch (pipe_status) {
        case DEVDRV_USBF_PIPE_IDLE:
            status = EP_COMPLETED;
            break;

        case DEVDRV_USBF_PIPE_WAIT:
            status = EP_PENDING;
            break;

        case DEVDRV_USBF_PIPE_DONE:
            usb0_function_stop_transfer(pipe);
            status = EP_COMPLETED;
            break;

        case DEVDRV_USBF_PIPE_NORES:
            status = EP_STALLED;
            break;

        case DEVDRV_USBF_PIPE_STALL:
            status = EP_STALLED;
            break;

        default:
            status = EP_PENDING;
    }

    return status;
}


/*************************************************************************/
void USBHAL::stallEndpoint(uint8_t endpoint)
{
    uint32_t pipe = EP2PIPE(endpoint);

    usb0_function_clear_pid_stall(pipe);
}


/*************************************************************************/
void USBHAL::unstallEndpoint(uint8_t endpoint)
{
    uint32_t pipe = EP2PIPE(endpoint);

    usb0_function_set_pid_stall( pipe );
}


/*************************************************************************/
bool USBHAL::getEndpointStallState(uint8_t endpoint)
{
    // No implemens
    return false;
}


/*************************************************************************/
#if 0   // No implements
void USBHAL::remoteWakeup(void)
{
}
#endif

/*************************************************************************/
void USBHAL::_usbisr(void)
{
    instance->usbisr();
}


/*************************************************************************/
void USBHAL::usbisr(void)
{
    uint16_t            int_sts0;
    uint16_t            int_sts1;
    uint16_t            int_sts2;
    uint16_t            int_sts3;
    uint16_t            int_enb0;
    uint16_t            int_enb2;
    uint16_t            int_enb3;
    uint16_t            int_enb4;
    volatile uint16_t   dumy_sts;


    int_sts0 = USB200.INTSTS0;

    if (!(int_sts0 & (
                USB_FUNCTION_BITVBINT |
                USB_FUNCTION_BITRESM  |
                USB_FUNCTION_BITSOFR  |
                USB_FUNCTION_BITDVST  |
                USB_FUNCTION_BITCTRT  |
                USB_FUNCTION_BITBEMP  |
                USB_FUNCTION_BITNRDY  |
                USB_FUNCTION_BITBRDY ))) {
        return;
    }

    int_sts1 = USB200.BRDYSTS;
    int_sts2 = USB200.NRDYSTS;
    int_sts3 = USB200.BEMPSTS;
    int_enb0 = USB200.INTENB0;
    int_enb2 = USB200.BRDYENB;
    int_enb3 = USB200.NRDYENB;
    int_enb4 = USB200.BEMPENB;

    if ((int_sts0 & USB_FUNCTION_BITRESM) &&
            (int_enb0 & USB_FUNCTION_BITRSME)) {
        USB200.INTSTS0 = (uint16_t)~USB_FUNCTION_BITRESM;
        RZA_IO_RegWrite_16(&USB200.INTENB0, 0, USB_INTENB0_RSME_SHIFT, USB_INTENB0_RSME);
        /*usb0_function_USB_FUNCTION_Resume();*/
        suspendStateChanged(1);
    } else if (
        (int_sts0 & USB_FUNCTION_BITVBINT) &&
        (int_enb0 & USB_FUNCTION_BITVBSE)) {
        USB200.INTSTS0 = (uint16_t)~USB_FUNCTION_BITVBINT;

        if (usb0_function_CheckVBUStaus() == DEVDRV_USBF_ON) {
            usb0_function_USB_FUNCTION_Attach();
        } else {
            usb0_function_USB_FUNCTION_Detach();
        }
    } else if (
        (int_sts0 & USB_FUNCTION_BITSOFR) &&
        (int_enb0 & USB_FUNCTION_BITSOFE)) {
        USB200.INTSTS0 = (uint16_t)~USB_FUNCTION_BITSOFR;
        SOF((USB200.FRMNUM & USB_FRMNUM_FRNM) >> USB_FRMNUM_FRNM_SHIFT);
    } else if (
        (int_sts0 & USB_FUNCTION_BITDVST) &&
        (int_enb0 & USB_FUNCTION_BITDVSE)) {
        USB200.INTSTS0 = (uint16_t)~USB_FUNCTION_BITDVST;
        switch (int_sts0 & USB_FUNCTION_BITDVSQ) {
            case USB_FUNCTION_DS_POWR:
                break;

            case USB_FUNCTION_DS_DFLT:
                /*****************************************************************************
                 * Function Name: usb0_function_USB_FUNCTION_BusReset
                 * Description  : This function is executed when the USB device is transitioned
                 *              : to POWERD_STATE. Sets the device descriptor according to the
                 *              : connection speed determined by the USB reset hand shake.
                 * Arguments    : none
                 * Return Value : none
                 *****************************************************************************/
                usb0_function_init_status();            /* memory clear */

#if 0
                /* You would program those steps in USBCallback_busReset
                 * if the system need the comment out steps.
                 */

                if (usb0_function_is_hispeed() == USB_FUNCTION_HIGH_SPEED) {
                    /* Device Descriptor reset */
                    usb0_function_ResetDescriptor(USB_FUNCTION_HIGH_SPEED);
                } else {
                    /* Device Descriptor reset */
                    usb0_function_ResetDescriptor(USB_FUNCTION_FULL_SPEED);
                }
#endif
                /* Default Control PIPE reset */
                /*****************************************************************************
                 * Function Name: usb0_function_ResetDCP
                 * Description  : Initializes the default control pipe(DCP).
                 * Outline      : Reset default control pipe
                 * Arguments    : none
                 * Return Value : none
                 *****************************************************************************/
                USB200.DCPCFG  = 0;
                USB200.DCPMAXP = 64;    /*TODO: This value is copied from sample*/

                USB200.CFIFOSEL  = (uint16_t)(USB_FUNCTION_BITMBW_8 | USB_FUNCTION_BITBYTE_LITTLE);
                USB200.D0FIFOSEL = (uint16_t)(USB_FUNCTION_BITMBW_8 | USB_FUNCTION_BITBYTE_LITTLE);
                USB200.D1FIFOSEL = (uint16_t)(USB_FUNCTION_BITMBW_8 | USB_FUNCTION_BITBYTE_LITTLE);

                busReset();
                break;

            case USB_FUNCTION_DS_ADDS:
                break;

            case USB_FUNCTION_DS_CNFG:
                break;

            case USB_FUNCTION_DS_SPD_POWR:
            case USB_FUNCTION_DS_SPD_DFLT:
            case USB_FUNCTION_DS_SPD_ADDR:
            case USB_FUNCTION_DS_SPD_CNFG:
                suspendStateChanged(0);
                /*usb0_function_USB_FUNCTION_Suspend();*/
                break;

            default:
                break;
        }
    } else if (
        (int_sts0 & USB_FUNCTION_BITBEMP) &&
        (int_enb0 & USB_FUNCTION_BITBEMP) &&
        ((int_sts3 & int_enb4) & g_usb0_function_bit_set[USB_FUNCTION_PIPE0])) {
        /* ==== BEMP PIPE0 ==== */
        usb0_function_BEMPInterrupt(int_sts3, int_enb4);
    } else if (
        (int_sts0 & USB_FUNCTION_BITBRDY) &&
        (int_enb0 & USB_FUNCTION_BITBRDY) &&
        ((int_sts1 & int_enb2) & g_usb0_function_bit_set[USB_FUNCTION_PIPE0])) {
        /* ==== BRDY PIPE0 ==== */
        usb0_function_BRDYInterrupt(int_sts1, int_enb2);
    } else if (
        (int_sts0 & USB_FUNCTION_BITNRDY) &&
        (int_enb0 & USB_FUNCTION_BITNRDY) &&
        ((int_sts2 & int_enb3) & g_usb0_function_bit_set[USB_FUNCTION_PIPE0])) {
        /* ==== NRDY PIPE0 ==== */
        usb0_function_NRDYInterrupt(int_sts2, int_enb3);
    } else if (
        (int_sts0 & USB_FUNCTION_BITCTRT) && (int_enb0 & USB_FUNCTION_BITCTRE)) {
        int_sts0 = USB200.INTSTS0;
        USB200.INTSTS0 = (uint16_t)~USB_FUNCTION_BITCTRT;

        if (((int_sts0 & USB_FUNCTION_BITCTSQ) == USB_FUNCTION_CS_RDDS) ||
                ((int_sts0 & USB_FUNCTION_BITCTSQ) == USB_FUNCTION_CS_WRDS) ||
                ((int_sts0 & USB_FUNCTION_BITCTSQ) == USB_FUNCTION_CS_WRND)) {

            /*EP0を再度作り上げる*/
            usb0_function_save_request();
            if ((USB200.INTSTS0 & USB_FUNCTION_BITVALID) && (
                        ((int_sts0 & USB_FUNCTION_BITCTSQ) == USB_FUNCTION_CS_RDDS) ||
                        ((int_sts0 & USB_FUNCTION_BITCTSQ) == USB_FUNCTION_CS_WRDS) ||
                        ((int_sts0 & USB_FUNCTION_BITCTSQ) == USB_FUNCTION_CS_WRND))) {
                /* New SETUP token received */
                /* Three dummy reads for cleearing interrupt requests */
                dumy_sts = USB200.INTSTS0;
                dumy_sts = USB200.INTSTS0;
                dumy_sts = USB200.INTSTS0;
                return;
            }
        }

        switch (int_sts0 & USB_FUNCTION_BITCTSQ) {
            case USB_FUNCTION_CS_IDST:
                if (g_usb0_function_TestModeFlag == DEVDRV_USBF_YES) {
                    /* ==== Test Mode ==== */
                    usb0_function_USB_FUNCTION_TestMode();
                }
                /* Needs not procedure in this state */
                break;

            case USB_FUNCTION_CS_RDDS:
                /* Reads a setup packet */
                EP0setupCallback();
                break;

            case USB_FUNCTION_CS_WRDS:
                /* Original code was the SetDescriptor was called */
                EP0setupCallback();
                break;

            case USB_FUNCTION_CS_WRND:
                EP0setupCallback();

                /*The EP0setupCallback should finish in successful */
                usb0_function_set_pid_buf(USB_FUNCTION_PIPE0);

                RZA_IO_RegWrite_16(&USB200.DCPCTR, 1, USB_DCPCTR_CCPL_SHIFT, USB_DCPCTR_CCPL);
                break;

            case USB_FUNCTION_CS_RDSS:
                RZA_IO_RegWrite_16(&USB200.DCPCTR, 1, USB_DCPCTR_CCPL_SHIFT, USB_DCPCTR_CCPL);
                break;

            case USB_FUNCTION_CS_WRSS:
                RZA_IO_RegWrite_16(&USB200.DCPCTR, 1, USB_DCPCTR_CCPL_SHIFT, USB_DCPCTR_CCPL);
                break;

            case USB_FUNCTION_CS_SQER:
                usb0_function_set_pid_stall(USB_FUNCTION_PIPE0);
                break;

            default:
                usb0_function_set_pid_stall(USB_FUNCTION_PIPE0);
                break;
        }
    } else if (
        (int_sts0 & USB_FUNCTION_BITBEMP) &&
        (int_enb0 & USB_FUNCTION_BITBEMP) &&
        (int_sts3 & int_enb4) ) {
        /* ==== BEMP PIPEx ==== */
        usb0_function_BEMPInterrupt(int_sts3, int_enb4);
    } else if (
        (int_sts0 & USB_FUNCTION_BITBRDY) &&
        (int_enb0 & USB_FUNCTION_BITBRDY) &&
        (int_sts1 & int_enb2) ) {
        /* ==== BRDY PIPEx ==== */
        usb0_function_BRDYInterrupt(int_sts1, int_enb2);
    } else if (
        (int_sts0 & USB_FUNCTION_BITNRDY) &&
        (int_enb0 & USB_FUNCTION_BITNRDY) &&
        (int_sts2 & int_enb3)) {
        /* ==== NRDY PIPEx ==== */
        usb0_function_NRDYInterrupt(int_sts2, int_enb3);
    } else {
        /* Do Nothing */
    }

    /* Three dummy reads for cleearing interrupt requests */
    dumy_sts = USB200.INTSTS0;
    dumy_sts = USB200.INTSTS1;
}

/******************************************************************************
 * Function Name: usb0_function_save_request
 * Description  : Retains the USB request information in variables.
 * Arguments    : none
 * Return Value : none
 *****************************************************************************/
void USBHAL::usb0_function_save_request(void)
{
    uint16_t *bufO = &setup_buffer[0];

    USB200.INTSTS0 = (uint16_t)~USB_FUNCTION_BITVALID;
    /*data[0] <= bmRequest, data[1] <= bmRequestType */
    *bufO++ = USB200.USBREQ;
    /*data[2] data[3] <= wValue*/
    *bufO++ = USB200.USBVAL;
    /*data[4] data[5] <= wIndex*/
    *bufO++ = USB200.USBINDX;
    /*data[6] data[6] <= wIndex*/
    *bufO++ = USB200.USBLENG;

#if defined(DEBUG_RZ_A1H)
    printf( "request: %.4xh\n", setup_buffer[0] );
#endif
}

/******************************************************************************
 * Function Name: usb0_function_BRDYInterrupt
 * Description  : Executes BRDY interrupt.
 * Arguments    : uint16_t status       ; BRDYSTS Register Value
 *              : uint16_t intenb       ; BRDYENB Register Value
 * Return Value : none
 *****************************************************************************/
void USBHAL::usb0_function_BRDYInterrupt(uint16_t status, uint16_t intenb)
{
    volatile uint16_t dumy_sts;
    uint16_t read_status;

    if ( ( status & g_usb0_function_bit_set[USB_FUNCTION_PIPE0] ) &&
            ( intenb & g_usb0_function_bit_set[USB_FUNCTION_PIPE0] ) ) {
        USB200.BRDYSTS = (uint16_t)~g_usb0_function_bit_set[USB_FUNCTION_PIPE0];
        RZA_IO_RegWrite_16(&USB200.CFIFOSEL, USB_FUNCTION_PIPE0, USB_CFIFOSEL_CURPIPE_SHIFT, USB_CFIFOSEL_CURPIPE);

        g_usb0_function_PipeDataSize[USB_FUNCTION_PIPE0] = g_usb0_function_data_count[USB_FUNCTION_PIPE0];

        read_status = usb0_function_read_buffer_c(USB_FUNCTION_PIPE0);

        g_usb0_function_PipeDataSize[USB_FUNCTION_PIPE0] -= g_usb0_function_data_count[USB_FUNCTION_PIPE0];

        switch (read_status) {
            case USB_FUNCTION_READING:      /* Continue of data read */
            case USB_FUNCTION_READEND:      /* End of data read */
                /* PID = BUF */
                usb0_function_set_pid_buf(USB_FUNCTION_PIPE0);

                /*callback*/
                EP0out();
                break;

            case USB_FUNCTION_READSHRT:     /* End of data read */
                usb0_function_disable_brdy_int(USB_FUNCTION_PIPE0);
                /* PID = BUF */
                usb0_function_set_pid_buf(USB_FUNCTION_PIPE0);

                /*callback*/
                EP0out();
                break;

            case USB_FUNCTION_READOVER:     /* FIFO access error */
                /* Buffer Clear */
                USB200.CFIFOCTR = USB_FUNCTION_BITBCLR;
                usb0_function_disable_brdy_int(USB_FUNCTION_PIPE0);
                /* Req Error */
                usb0_function_set_pid_stall(USB_FUNCTION_PIPE0);

                /*callback*/
                EP0out();
                break;

            case DEVDRV_USBF_FIFOERROR:     /* FIFO access error */
            default:
                usb0_function_disable_brdy_int(USB_FUNCTION_PIPE0);
                /* Req Error */
                usb0_function_set_pid_stall(USB_FUNCTION_PIPE0);
                break;
        }
    } else {
        /**********************************************************************
         * Function Name: usb0_function_brdy_int
         * Description  : Executes BRDY interrupt(USB_FUNCTION_PIPE1-9).
         *              : According to the pipe that interrupt is generated in,
         *              : reads/writes buffer allocated in the pipe.
         *              : This function is executed in the BRDY
         *              : interrupt handler.  This function
         *              : clears BRDY interrupt status and BEMP
         *              : interrupt status.
         * Arguments    : uint16_t Status       ; BRDYSTS Register Value
         *              : uint16_t Int_enbl     ; BRDYENB Register Value
         * Return Value : none
         *********************************************************************/
        /*usb0_function_brdy_int(status, intenb);*/
        /* copied from usb0_function_intrn.c */
        uint32_t int_sense = 0;
        uint16_t pipe;
        uint16_t pipebit;
        uint16_t ep;

        for (pipe = USB_FUNCTION_PIPE1; pipe <= USB_FUNCTION_MAX_PIPE_NO; pipe++) {
            pipebit = g_usb0_function_bit_set[pipe];

            if ((status & pipebit) && (intenb & pipebit)) {
                USB200.BRDYSTS = (uint16_t)~pipebit;
                USB200.BEMPSTS = (uint16_t)~pipebit;
                if ((g_usb0_function_PipeTbl[pipe] & USB_FUNCTION_FIFO_USE) == USB_FUNCTION_D0FIFO_DMA) {
                    if (g_usb0_function_DmaStatus[USB_FUNCTION_D0FIFO] != USB_FUNCTION_DMA_READY) {
                        /*now, DMA is not supported*/
                        usb0_function_dma_interrupt_d0fifo(int_sense);
                    }

                    if (RZA_IO_RegRead_16(
                                &g_usb0_function_pipecfg[pipe], USB_PIPECFG_BFRE_SHIFT, USB_PIPECFG_BFRE) == 0) {
                        /*now, DMA is not supported*/
                        usb0_function_read_dma(pipe);
                        usb0_function_disable_brdy_int(pipe);
                    } else {
                        USB200.D0FIFOCTR = USB_FUNCTION_BITBCLR;
                        g_usb0_function_pipe_status[pipe] = DEVDRV_USBF_PIPE_DONE;
                    }
                } else if ((g_usb0_function_PipeTbl[pipe] & USB_FUNCTION_FIFO_USE) == USB_FUNCTION_D1FIFO_DMA) {
                    if (g_usb0_function_DmaStatus[USB_FUNCTION_D1FIFO] != USB_FUNCTION_DMA_READY) {
                        /*now, DMA is not supported*/
                        usb0_function_dma_interrupt_d1fifo(int_sense);
                    }

                    if (RZA_IO_RegRead_16(
                                &g_usb0_function_pipecfg[pipe], USB_PIPECFG_BFRE_SHIFT, USB_PIPECFG_BFRE) == 0) {
                        /*now, DMA is not supported*/
                        usb0_function_read_dma(pipe);
                        usb0_function_disable_brdy_int(pipe);
                    } else {
                        USB200.D1FIFOCTR = USB_FUNCTION_BITBCLR;
                        g_usb0_function_pipe_status[pipe] = DEVDRV_USBF_PIPE_DONE;
                    }
                } else {
                    ep = (g_usb0_function_pipecfg[pipe] & USB_PIPECFG_EPNUM) >> USB_PIPECFG_EPNUM_SHIFT;
                    ep <<= 1;
                    ep += (RZA_IO_RegRead_16(&g_usb0_function_pipecfg[pipe], USB_PIPECFG_DIR_SHIFT, USB_PIPECFG_DIR) == 0)?
                          (0): (1);
                    EPx_read_status = DEVDRV_USBF_PIPE_WAIT;
                    (instance->*(epCallback[ep - 2])) ();
                    EPx_read_status = DEVDRV_USBF_PIPE_DONE;
                }
            }
        }
    }
    /* Three dummy reads for clearing interrupt requests */
    dumy_sts = USB200.BRDYSTS;
}

/******************************************************************************
 * Function Name: usb0_function_NRDYInterrupt
 * Description  : Executes NRDY interrupt.
 * Arguments    : uint16_t status       ; NRDYSTS Register Value
 *              : uint16_t intenb       ; NRDYENB Register Value
 * Return Value : none
 *****************************************************************************/
void USBHAL::usb0_function_NRDYInterrupt(uint16_t status, uint16_t intenb)
{
    volatile uint16_t dumy_sts;

    if ((status & g_usb0_function_bit_set[USB_FUNCTION_PIPE0]) &&
            (intenb & g_usb0_function_bit_set[USB_FUNCTION_PIPE0])) {
        USB200.NRDYSTS = (uint16_t)~g_usb0_function_bit_set[USB_FUNCTION_PIPE0];
    } else {
        usb0_function_nrdy_int(status, intenb);
    }
    /* Three dummy reads for clearing interrupt requests */
    dumy_sts = USB200.NRDYSTS;
}


/******************************************************************************
 * Function Name: usb0_function_BEMPInterrupt
 * Description  : Executes BEMP interrupt.
 * Arguments    : uint16_t status       ; BEMPSTS Register Value
 *              : uint16_t intenb       ; BEMPENB Register Value
 * Return Value : none
 *****************************************************************************/
void USBHAL::usb0_function_BEMPInterrupt(uint16_t status, uint16_t intenb)
{
    volatile uint16_t dumy_sts;

    if ((status & g_usb0_function_bit_set[USB_FUNCTION_PIPE0]) &&
            (intenb & g_usb0_function_bit_set[USB_FUNCTION_PIPE0])) {
        USB200.BEMPSTS = (uint16_t)~g_usb0_function_bit_set[USB_FUNCTION_PIPE0];
        RZA_IO_RegWrite_16(&USB200.CFIFOSEL, USB_FUNCTION_PIPE0, USB_CFIFOSEL_CURPIPE_SHIFT, USB_CFIFOSEL_CURPIPE);
        /*usb0_function_write_buffer_c(USB_FUNCTION_PIPE0);*/
        printf("<0>");
        EP0in();
    } else {
        usb0_function_bemp_int(status, intenb);
    }
    /* Three dummy reads for clearing interrupt requests */
    dumy_sts = USB200.BEMPSTS;
}

/*************************************************************************/
#endif
/*************************************************************************/
/*EOF*/
