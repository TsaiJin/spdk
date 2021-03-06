/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NVMF_SESSION_H
#define NVMF_SESSION_H

#include <stdint.h>
#include <stdbool.h>

#include "request.h"
#include "spdk/nvmf_spec.h"
#include "spdk/queue.h"

/*
 * This structure maintains local NVMf library specific connection
 * state that includes an opaque pointer back to its parent fabric
 * transport connection context.
 */
struct nvmf_connection_entry {
	void *fabric_conn;
	int is_aq_conn;

	TAILQ_ENTRY(nvmf_connection_entry) entries;
};

/* define a virtual controller limit to the number of QPs supported */
#define MAX_SESSION_IO_QUEUES 64

/*
 * This structure maintains the NVMf virtual controller session
 * state. Each NVMf session permits some number of connections.
 * At least one admin connection and additional IOQ connections.
 */
struct nvmf_session {
	struct spdk_nvmf_subsystem *subsys;

	uint16_t	cntlid;
	uint32_t	max_io_queues; /* maximum supported by backend NVMe library */
	int		active_queues;
	int		is_valid;
	struct {
		union spdk_nvme_cap_register	cap;
		union spdk_nvme_vs_register	vs;
		union spdk_nvme_cc_register	cc;
		union spdk_nvme_csts_register	csts;
	} vcprop; /* virtual controller properties */
	struct spdk_nvme_ctrlr_data	vcdata; /* virtual controller data */

	TAILQ_HEAD(connection_q, nvmf_connection_entry) connections;
	int num_connections;
	int max_connections_allowed;

	struct spdk_nvmf_request *aer_req;

	TAILQ_ENTRY(nvmf_session) entries;
};

struct nvmf_session *
nvmf_connect(void *fabric_conn,
	     struct spdk_nvmf_fabric_connect_cmd *connect,
	     struct spdk_nvmf_fabric_connect_data *connect_data,
	     struct spdk_nvmf_fabric_connect_rsp *response);

void
nvmf_disconnect(void *fabric_conn, struct nvmf_session *session);

void
nvmf_init_session_properties(struct nvmf_session *session, int aq_depth);

void
nvmf_property_get(struct nvmf_session *session,
		  struct spdk_nvmf_fabric_prop_get_cmd *cmd,
		  struct spdk_nvmf_fabric_prop_get_rsp *response);

void
nvmf_property_set(struct nvmf_session *session,
		  struct spdk_nvmf_fabric_prop_set_cmd *cmd,
		  struct spdk_nvmf_fabric_prop_set_rsp *response,
		  bool *shutdown);

void
nvmf_check_io_completions(struct nvmf_session *session);

void
nvmf_check_admin_completions(struct nvmf_session *session);

#endif
