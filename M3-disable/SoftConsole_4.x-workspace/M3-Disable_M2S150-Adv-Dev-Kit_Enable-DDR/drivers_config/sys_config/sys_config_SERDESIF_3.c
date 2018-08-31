/*=============================================================*/
/* Created by Microsemi SmartDesign Mon May 08 19:13:33 2017   */
/*                                                             */
/* Warning: Do not modify this file, it may lead to unexpected */
/*          functional failures in your design.                */
/*                                                             */
/*=============================================================*/

/*-------------------------------------------------------------*/
/* SERDESIF_3 Initialization                                   */
/*-------------------------------------------------------------*/

#include <stdint.h>
#include "../../CMSIS/sys_init_cfg_types.h"

const cfg_addr_value_pair_t g_m2s_serdes_3_config[] =
{
    { (uint32_t*)( 0x40034000 + 0x2028 ), 0x40F } /* SYSTEM_CONFIG_PHY_MODE_1 */ ,
    { (uint32_t*)( 0x40034000 + 0x1998 ), 0x30 } /* LANE2_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x40034000 + 0x1800 ), 0x80 } /* LANE2_CR0 */ ,
    { (uint32_t*)( 0x40034000 + 0x1804 ), 0x20 } /* LANE2_ERRCNT_DEC */ ,
    { (uint32_t*)( 0x40034000 + 0x1808 ), 0xF8 } /* LANE2_RXIDLE_MAX_ERRCNT_THR */ ,
    { (uint32_t*)( 0x40034000 + 0x180c ), 0x80 } /* LANE2_IMPED_RATIO */ ,
    { (uint32_t*)( 0x40034000 + 0x1814 ), 0x29 } /* LANE2_PLL_M_N */ ,
    { (uint32_t*)( 0x40034000 + 0x1818 ), 0x20 } /* LANE2_CNT250NS_MAX */ ,
    { (uint32_t*)( 0x40034000 + 0x1824 ), 0x80 } /* LANE2_TX_AMP_RATIO */ ,
    { (uint32_t*)( 0x40034000 + 0x1828 ), 0x15 } /* LANE2_TX_PST_RATIO */ ,
    { (uint32_t*)( 0x40034000 + 0x1830 ), 0x10 } /* LANE2_ENDCALIB_MAX */ ,
    { (uint32_t*)( 0x40034000 + 0x1834 ), 0x38 } /* LANE2_CALIB_STABILITY_COUNT */ ,
    { (uint32_t*)( 0x40034000 + 0x183c ), 0x70 } /* LANE2_RX_OFFSET_COUNT */ ,
    { (uint32_t*)( 0x40034000 + 0x19d4 ), 0x2 } /* LANE2_GEN1_TX_PLL_CCP */ ,
    { (uint32_t*)( 0x40034000 + 0x19d8 ), 0x22 } /* LANE2_GEN1_RX_PLL_CCP */ ,
    { (uint32_t*)( 0x40034000 + 0x1998 ), 0x0 } /* LANE2_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x40034000 + 0x1a00 ), 0x1 } /* LANE2_UPDATE_SETTINGS */ ,
    { (uint32_t*)( 0x40034000 + 0x2028 ), 0xF0F } /* SYSTEM_CONFIG_PHY_MODE_1 */ 
};

