
#include <grlib/ambapp_bus_grlib.h>
#include <grlib/ambapp_bus.h>
#include <grlib/ambapp_ids.h>

#define DRIVER_AMBAPP_GAISLER_GRSPW2_ID	DRIVER_AMBAPP_ID(VENDOR_GAISLER, GAISLER_SPW2)

/* B1553RT driver configuration (optional) */
struct drvmgr_key grlib_drv_res_b1553rt0[] =
{
#ifdef SET_B1553RT_FREQ
	{"coreFreq", DRVMGR_KT_INT, {(unsigned int)SET_B1553RT_FREQ}},
#endif
	DRVMGR_KEY_EMPTY
};

/* GRPCI driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grpci0[] =
{
/*	{"byteTwisting", DRVMGR_KT_INT, {(unsigned int)1}}, */
	DRVMGR_KEY_EMPTY
};

/* GRPCI2 driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grpci2_0[] =
{
#if 0
	{"INTA#", DRVMGR_KT_INT, {(unsigned int)3}},
	{"INTB#", DRVMGR_KT_INT, {(unsigned int)3}},
	{"INTC#", DRVMGR_KT_INT, {(unsigned int)3}},
	{"INTD#", DRVMGR_KT_INT, {(unsigned int)3}},
#endif
	DRVMGR_KEY_EMPTY
};

/* GRGPIO0 driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grgpio0[] =
{
#if 0
	{"nBits", DRVMGR_KT_INT, {(unsigned int)24}},
#endif
	{"int1", DRVMGR_KT_INT,  {(unsigned int)1}},
	{"ptr2", DRVMGR_KT_POINTER,  {(unsigned int)0x23334445}},
	{"str3", DRVMGR_KT_STRING,  {(unsigned int)"STRING_ValUe"}},
	DRVMGR_KEY_EMPTY
};

/* GRGPIO1 driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grgpio1[] =
{
	{"nBits", DRVMGR_KT_INT, {(unsigned int)8}},
	DRVMGR_KEY_EMPTY
};

/* GRGPIO1 driver configuration (optional) */
struct drvmgr_key grlib_drv_res_spictrl0[] =
{
#ifdef SPICTRL_SLV_SEL_FUNC
	{"slvSelFunc", DRVMGR_KT_POINTER, {(unsigned int)SPICTRL_SLV_SEL_FUNC}},
#endif
	DRVMGR_KEY_EMPTY
};

#ifdef TEST_NGMP /* Assign custom GRSPW buffer descriptor table base address */
/* GRSPW[0] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt0[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x01000000}},
/*	{"nDMA", DRVMGR_KT_INT, {(unsigned int)1}},*/
	DRVMGR_KEY_EMPTY
};

/* GRSPW[1] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt1[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x01002000}},
	DRVMGR_KEY_EMPTY
};

/* GRSPW[2] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt2[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x01004000}},
	DRVMGR_KEY_EMPTY
};

/* GRSPW[3] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt3[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x01006000}},
	DRVMGR_KEY_EMPTY
};

/* GRSPW[4] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt4[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x01008000}},
	DRVMGR_KEY_EMPTY
};

/* GRSPW[5] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt5[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x0100a000}},
	DRVMGR_KEY_EMPTY
};

/* GRSPW[6] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt6[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x0100c000}},
	DRVMGR_KEY_EMPTY
};

/* GRSPW[7] GRSPW packet driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grspwPkt7[] =
{
	{"bdDmaArea", DRVMGR_KT_INT, {(unsigned int)0x0100e000}},
	DRVMGR_KEY_EMPTY
};
#endif

/* If RTEMS_DRVMGR_STARTUP is defined we override the "weak defaults" that
 * is defined by the LEON3 BSP.
 */
struct drvmgr_bus_res grlib_drv_resources =
{
	.next = NULL,
	.resource = {
	{DRIVER_AMBAPP_GAISLER_GRPCI2_ID, 0, &grlib_drv_res_grpci2_0[0]},
/*
	{DRIVER_AMBAPP_GAISLER_B1553RT_ID, 0, &grlib_drv_res_b1553rt0[0]},
	{DRIVER_AMBAPP_GAISLER_GRPCI_ID, 0, &grlib_drv_res_grpci0[0]},
	{DRIVER_AMBAPP_GAISLER_SPICTRL_ID, 0, &grlib_drv_res_spictrl0[0]},
*/
	{DRIVER_AMBAPP_GAISLER_GRGPIO_ID, 0, &grlib_drv_res_grgpio0[0]},
#ifdef TEST_NGMP
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 0, &grlib_drv_res_grspwPkt0[0]},
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 1, &grlib_drv_res_grspwPkt1[0]},
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 2, &grlib_drv_res_grspwPkt2[0]},
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 3, &grlib_drv_res_grspwPkt3[0]},
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 4, &grlib_drv_res_grspwPkt4[0]},
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 5, &grlib_drv_res_grspwPkt5[0]},
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 6, &grlib_drv_res_grspwPkt6[0]},
	{DRIVER_AMBAPP_GAISLER_GRSPW2_ID, 7, &grlib_drv_res_grspwPkt7[0]},
#endif
	DRVMGR_RES_EMPTY
	}
};

#ifndef RTEMS_DRVMGR_STARTUP
struct grlib_config grlib_bus_config = 
{
	&ambapp_plb,		/* AMBAPP bus setup */
	&grlib_drv_resources,	/* Driver configuration */
};
#endif

/* LEON3 specific system init */
void system_init2(void)
{
#ifndef RTEMS_DRVMGR_STARTUP
	/* Register GRLIB root bus */
	ambapp_grlib_root_register(&grlib_bus_config);
#endif
}
