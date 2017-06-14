/*
 * Copyright (c) 2014-2016 Cesanta Software Limited
 * All rights reserved
 */

#include "mgos_ota_http_client.h"

#include "common/cs_dbg.h"
#include "fw/src/mgos_hal.h"
#include "fw/src/mgos_mongoose.h"
#include "fw/src/mgos_sys_config.h"
#include "fw/src/mgos_timers.h"
#include "fw/src/mgos_utils.h"

#if MGOS_ENABLE_UPDATER

static void fw_download_handler(struct mg_connection *c, int ev, void *p,
                                void *user_data) {
  struct mbuf *io = &c->recv_mbuf;
  struct update_context *ctx = (struct update_context *) user_data;
  int res = 0;
  struct mg_str *loc;
  (void) p;

  switch (ev) {
    case MG_EV_CONNECT: {
      int result = *((int *) p);
      if (result != 0) LOG(LL_ERROR, ("connect error: %d", result));
      break;
    }
    case MG_EV_RECV: {
      if (ctx->file_size == 0) {
        LOG(LL_DEBUG, ("Looking for HTTP header"));
        struct http_message hm;
        int parsed = mg_parse_http(io->buf, io->len, &hm, 0);
        if (parsed <= 0) {
          return;
        }
        if (hm.resp_code != 200) {
          if (hm.resp_code == 304) {
            ctx->result = 1;
            ctx->need_reboot = false;
            ctx->status_msg = "Not Modified";
            updater_finish(ctx);
          } else if ((hm.resp_code == 301 || hm.resp_code == 302) &&
                     (loc = mg_get_http_header(&hm, "Location")) != NULL) {
            /* NUL-terminate the URL. Every header must be followed by \r\n,
             * so there is deifnitely space there. */
            ((char *) loc->p)[loc->len] = '\0';
            /* We were told to look elsewhere. Detach update context from this
             * connection so that it doesn't get finalized when it's closed. */
            mgos_ota_http_start(ctx, loc->p);
            c->user_data = NULL;
          } else {
            ctx->result = -hm.resp_code;
            ctx->need_reboot = false;
            ctx->status_msg = "Invalid HTTP response code";
            updater_finish(ctx);
          }
          c->flags |= MG_F_CLOSE_IMMEDIATELY;
          return;
        }
        if (hm.body.len != 0) {
          LOG(LL_DEBUG, ("HTTP header: file size: %d", (int) hm.body.len));
          if (hm.body.len == (size_t) ~0) {
            LOG(LL_ERROR, ("Invalid content-length, perhaps chunked-encoding"));
            ctx->status_msg =
                "Invalid content-length, perhaps chunked-encoding";
            c->flags |= MG_F_CLOSE_IMMEDIATELY;
            break;
          } else {
            ctx->file_size = hm.body.len;
          }

          mbuf_remove(io, parsed);
        }
      }

      if (io->len != 0) {
        res = updater_process(ctx, io->buf, io->len);
        mbuf_remove(io, io->len);

        if (res == 0) {
          if (is_write_finished(ctx)) res = updater_finalize(ctx);
          if (res == 0) {
            /* Need more data, everything is OK */
            break;
          }
        }

        if (res < 0) {
          /* Error */
          LOG(LL_ERROR, ("Update error: %d %s", ctx->result, ctx->status_msg));
        }
        c->flags |= MG_F_CLOSE_IMMEDIATELY;
      }
      break;
    }
    case MG_EV_CLOSE: {
      if (ctx == NULL) break;

      if (is_write_finished(ctx)) updater_finalize(ctx);

      if (!is_update_finished(ctx)) {
        /* Update failed or connection was terminated by server */
        if (ctx->status_msg == NULL) ctx->status_msg = "Update failed";
        ctx->result = -1;
      } else if (is_reboot_required(ctx)) {
        LOG(LL_INFO, ("Rebooting device"));
        mgos_system_restart_after(100);
      }
      updater_finish(ctx);
      updater_context_free(ctx);
      c->user_data = NULL;
      break;
    }
  }
}

void mgos_ota_http_start(struct update_context *ctx, const char *url) {
  LOG(LL_INFO, ("Update URL: %s, ct: %d, isv? %d", url,
                ctx->fctx.commit_timeout, ctx->ignore_same_version));

  struct mg_connect_opts opts;
  memset(&opts, 0, sizeof(opts));

#if MG_ENABLE_SSL
  if (strlen(url) > 8 && strncmp(url, "https://", 8) == 0) {
    opts.ssl_server_name = get_cfg()->update.ssl_server_name;
    opts.ssl_ca_cert = get_cfg()->update.ssl_ca_file;
    opts.ssl_cert = get_cfg()->update.ssl_client_cert_file;
  }
#endif

  char ehb[150];
  char *extra_headers = ehb;
  const struct sys_ro_vars *rv = get_ro_vars();
  mg_asprintf(&extra_headers, sizeof(ehb),
              "X-MGOS-Device-ID: %s %s\r\n"
              "X-MGOS-FW-Version: %s %s %s\r\n",
              (get_cfg()->device.id ? get_cfg()->device.id : "-"),
              rv->mac_address, rv->arch, rv->fw_version, rv->fw_id);

  struct mg_connection *c = mg_connect_http_opt(
      mgos_get_mgr(), fw_download_handler, ctx, opts, url, extra_headers, NULL);

  if (extra_headers != ehb) free(extra_headers);

  if (c == NULL) {
    LOG(LL_ERROR, ("Failed to connect to %s", url));
    ctx->result = -10;
    ctx->need_reboot = false;
    ctx->status_msg = "Failed to connect";
    updater_finish(ctx);
    return;
  }

  ctx->nc = c;
}

static void mgos_ota_timer_cb(void *arg) {
  struct sys_config_update *scu = &get_cfg()->update;
  if (scu->url == NULL) return;
  struct update_context *ctx = updater_context_create();
  if (ctx == NULL) return;
  ctx->ignore_same_version = true;
  ctx->fctx.commit_timeout = scu->commit_timeout;
  mgos_ota_http_start(ctx, scu->url);

  (void) arg;
}

bool mgos_ota_http_client_init(void) {
  struct sys_config_update *scu = &get_cfg()->update;
  if (scu->url != NULL && scu->interval > 0) {
    LOG(LL_INFO,
        ("Updates from %s, every %d seconds", scu->url, scu->interval));
    mgos_set_timer(scu->interval * 1000, true /* repeat */, mgos_ota_timer_cb,
                   scu->url);
  }
  return true;
}

#endif /* MGOS_ENABLE_UPDATER */
