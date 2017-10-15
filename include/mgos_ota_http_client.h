/*
 * Copyright (c) 2014-2016 Cesanta Software Limited
 * All rights reserved
 *
 * An updater implementation that fetches FW from the given URL.
 */

#ifndef CS_MOS_LIBS_OTA_HTTP_CLIENT_SRC_MGOS_OTA_HTTP_CLIENT_H_
#define CS_MOS_LIBS_OTA_HTTP_CLIENT_SRC_MGOS_OTA_HTTP_CLIENT_H_

#include <stdbool.h>

#include "mgos_updater_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if MGOS_ENABLE_UPDATER
void mgos_ota_http_start(struct update_context *ctx, const char *url);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_MOS_LIBS_OTA_HTTP_CLIENT_SRC_MGOS_OTA_HTTP_CLIENT_H_ */
