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

/*********************************************************************
 * This file contains PM master related data structures and
 * functions for accessing them. Also, each PM masters have defined
 * data structures used for tracking it's requests for each slave's
 * capabilities/states.
 *********************************************************************/

#ifndef PM_MASTER_H_
#define PM_MASTER_H_

#include "pm_slave.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_node.h"
#include "pm_power.h"
#include "xil_types.h"

typedef struct PmMaster PmMaster;
typedef struct PmRequirement PmRequirement;

/*********************************************************************
 * Macros
 ********************************************************************/

/* Pm Master request info masks */
#define PM_MASTER_WAKEUP_REQ_MASK   0x1U
#define PM_MASTER_USING_SLAVE_MASK  0x2U

/* Maximum number of masters currently supported */
#define PM_MASTER_MAX               3U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmRequirement - structure for tracking requirements of a master for the slave
 *              setting. One structure should be statically assigned for each
 *              possible combination of master/slave, because dynamic memory
 *              allocation cannot be used.
 * @slave       Pointer to the slave structure
 * @master      Pointer to the master structure
 * @nextSlave   Pointer to the master's requirement for a next slave in the list
 * @nextMaster  Pointer to the requirement of a next master that uses the slave
 * @defaultReq  Default requirements of a master - requirements for slave
 *              capabilities without which master's primary processor cannot
 *              switch to active state.
 * @currReq     Currently holding requirements of a master for this slave
 * @nextReq     Requirements of a master to be configured when it changes the
 *              state (after it goes to sleep or before it gets awake)
 * @latencyReq  Latency requirements of master for the slave's transition time
 *              from any to its maximum (highest id) state
 * @info        Contains information about master's request - a bit for
 *              encoding has master requested or released node, and a bit to
 *              encode has master requested a wake-up of this slave.
 */
typedef struct PmRequirement {
	PmSlave* slave;
	PmMaster* master;
	PmRequirement* nextSlave;
	PmRequirement* nextMaster;
	const u32 defaultReq;
	u32 currReq;
	u32 nextReq;
	u32 latencyReq;
	u8 info;
} PmRequirement;

/**
 * PmSuspendRequest() - For tracking information about request suspend being
 *                      processed at the moment
 * @initiator   Master which has requested suspend
 * @acknowledge Acknowledge argument provided with the request suspend call
 */
typedef struct {
	const PmMaster* initiator;
	u32 acknowledge;
} PmSuspendRequest;

/**
 * PmMaster - contains PM master related informations
 * @procs       Pointer to the array of processors within the master
 * @reqs        Pointer to the master's list of requirements for slaves'
 *              capabilities. For every slave that the master can use there has
 *              to be a dedicated requirements structure
 * @ipiMask     Mask dedicated to the master in IPI registers
 * @pmuBuffer   IPI buffer address into which PMU can write (PMU's buffer)
 * @buffer      IPI buffer address into which this master can write
 *              (master's buffer)
 * @nid         Placeholder nodeId - used to encode request suspend for group of
 *              processors sharing the same communication channel. When PM
 *              receives this nid as request suspend argument, it initiates
 *              init suspend to the master. At the PU, in init_suspend_cb
 *              implementation, the request is mappend to actual suspend of all
 *              processors in the PU. In RPU case, this data could be
 *              initialized from PCW, based on RPU configuration.
 * @procsCnt    Number of processors within the master
 * @permissions ORed ipi masks of masters which this master is allowed to
 *              request to suspend (to be updated based on specific
 *              configuration, by default all masters should be able to request
 *              any other master to suspend)
 * @suspendRequest Captures info about the ongoing suspend request (this master
 *              is the target which suppose to suspend). At any moment only
 *              one suspend request can be active for one target/master
 */
typedef struct PmMaster {
	PmSuspendRequest suspendRequest;
	PmProc* const procs;
	PmRequirement* reqs;
	const u32 ipiMask;
	const u32 pmuBuffer;
	const u32 buffer;
	u32 permissions;
	PmNodeId nid;
	const u8 procsCnt;
} PmMaster;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmMaster pmMasterApu_g;
extern PmMaster pmMasterRpu0_g;
extern PmMaster pmMasterRpu1_g;

extern PmMaster *const pmAllMasters[PM_MASTER_MAX];

/*********************************************************************
 * Function declarations
 ********************************************************************/
/* Get functions */
const PmMaster* PmGetMasterByIpiMask(const u32 mask);

PmProc* PmGetProcByWfiStatus(const u32 mask);
PmProc* PmGetProcByWakeStatus(const u32 mask);
PmProc* PmGetProcByNodeId(const PmNodeId nodeId);
PmProc* PmGetProcOfThisMaster(const PmMaster* const master,
			      const PmNodeId nodeId);
PmProc* PmGetProcOfOtherMaster(const PmMaster* const master,
			       const PmNodeId nodeId);
PmRequirement* PmGetRequirementForSlave(const PmMaster* const master,
			      const PmNodeId nodeId);

/* Requirements related functions */
int PmRequirementSchedule(PmRequirement* const masterReq, const u32 caps);
int PmRequirementUpdate(PmRequirement* const masterReq, const u32 caps);

void PmRequirementInit(void);

/* Notify master by a primary core when changing state */
int PmMasterNotify(PmMaster* const master, const PmProcEvent event);

/* Call at initialization to enable all masters' IPI interrupts */
void PmEnableAllMasterIpis(void);

/* Call when FPD goes down to enable GIC Proxy interrupts */
void PmEnableProxyWake(PmMaster* const master);

bool PmCanRequestSuspend(const PmMaster* const reqMaster,
			 const PmMaster* const respMaster);
bool PmIsRequestedToSuspend(const PmMaster* const master);

int PmMasterSuspendAck(PmMaster* const mst, const int response);

PmMaster* PmMasterGetPlaceholder(const PmNodeId nodeId);

void PmSetupInitialMasterRequirements(void);

#endif
