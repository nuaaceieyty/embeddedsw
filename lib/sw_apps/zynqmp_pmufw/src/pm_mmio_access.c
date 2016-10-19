/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

#include "pm_master.h"
#include "pm_mmio_access.h"
#include "crl_apb.h"
#include "crf_apb.h"
#include "pmu_iomodule.h"

#define PM_MMIO_IOU_SLCR_BASE  0xFF180000
#define PM_MMIO_CSU_BASE       0xFFCA0000

/**
 * PmAccessRegion - Structure containing information about memory access
                    permissions
 * @startAddr   Starting address of the memory region
 * @endAddr     Ending address of the memory region
 * @access      Access control bitmask (1 bit per master, see 'pmAllMasters')
 */
typedef struct PmAccessRegion {
	const u32 startAddr;
	const u32 endAddr;
	const u32 access;
} PmAccessRegion;

static const PmAccessRegion pmAccessTable[] = {
	/* Module clock controller full power domain (CRF_APB) */
	{
		.startAddr = CRF_APB_BASEADDR + 0x20,
		.endAddr = CRF_APB_BASEADDR + 0x63,
		.access = IPI_PMU_0_IER_APU_MASK |
			  IPI_PMU_0_IER_RPU_0_MASK |
			  IPI_PMU_0_IER_RPU_1_MASK,
	},

	{
		.startAddr = CRF_APB_BASEADDR + 0x70,
		.endAddr = CRF_APB_BASEADDR + 0x7b,
		.access = IPI_PMU_0_IER_APU_MASK |
			  IPI_PMU_0_IER_RPU_0_MASK |
			  IPI_PMU_0_IER_RPU_1_MASK,
	},

	{
		.startAddr = CRF_APB_BASEADDR + 0x84,
		.endAddr = CRF_APB_BASEADDR + 0xbf,
		.access = IPI_PMU_0_IER_APU_MASK |
			  IPI_PMU_0_IER_RPU_0_MASK |
			  IPI_PMU_0_IER_RPU_1_MASK,
	},

	/* Module clock controller low power domain (CRL_APB) */
	{
		.startAddr = CRL_APB_BASEADDR + 0x20,
		.endAddr = CRL_APB_BASEADDR + 0x8c,
		.access = IPI_PMU_0_IER_APU_MASK |
			  IPI_PMU_0_IER_RPU_0_MASK |
			  IPI_PMU_0_IER_RPU_1_MASK,
	},

	{
		.startAddr = CRL_APB_BASEADDR + 0xa4,
		.endAddr = CRL_APB_BASEADDR + 0xa7,
		.access = IPI_PMU_0_IER_APU_MASK |
			  IPI_PMU_0_IER_RPU_0_MASK |
			  IPI_PMU_0_IER_RPU_1_MASK,
	},

	{
		.startAddr = CRL_APB_BASEADDR + 0xb4,
		.endAddr = CRL_APB_BASEADDR + 0x12b,
		.access = IPI_PMU_0_IER_APU_MASK |
			  IPI_PMU_0_IER_RPU_0_MASK |
			  IPI_PMU_0_IER_RPU_1_MASK,
	},

	/* PMU's global Power Status register*/
	{
		.startAddr = PMU_GLOBAL_PWR_STATE,
		.endAddr = PMU_GLOBAL_PWR_STATE,
		.access = IPI_PMU_0_IER_APU_MASK,
	},

	/* PMU's global gen storage */
	{
		.startAddr = PMU_GLOBAL_GLOBAL_GEN_STORAGE0,
		.endAddr = PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7,
		.access = IPI_PMU_0_IER_APU_MASK |
			  IPI_PMU_0_IER_RPU_0_MASK |
			  IPI_PMU_0_IER_RPU_1_MASK,
	},

	/* IOU SLCR Registers required for Linux */
	{
		.startAddr = PM_MMIO_IOU_SLCR_BASE + 0x300,
		.endAddr = PM_MMIO_IOU_SLCR_BASE + 0x524,
		.access = IPI_PMU_0_IER_APU_MASK,
	},

	/* CSU Device IDCODE and Version Registers */
	{
		.startAddr = PM_MMIO_CSU_BASE + 0x40,
		.endAddr = PM_MMIO_CSU_BASE + 0x44,
		.access = IPI_PMU_0_IER_APU_MASK,
	},
};

/**
 * PmGetMmioAccess() - Retrieve access info for a particular address
 * @master     Master who requests access permission
 * @address    Address to write/read
 *
 * @return     Return true if master's IPI bit was present in the access region
 *             table
 */
bool PmGetMmioAccess(const PmMaster *const master, const u32 address)
{
	u32 i;
	bool permission = false;

	if (NULL == master) {
		goto done;
	}

	for (i = 0U; i < ARRAY_SIZE(pmAccessTable); i++) {
		if ((address >= pmAccessTable[i].startAddr) &&
		    (address <= pmAccessTable[i].endAddr)) {
			permission = !!(pmAccessTable[i].access &
					master->ipiMask);
			break;
		}
	}

done:
	return permission;
}
