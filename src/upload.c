#include "protocol-constants.h"
#include "requests.h"
#include "trg-client.h"
#include "upload.h"
#include "util.h"
#include "trg-main-window.h"

static gboolean upload_complete_callback(gpointer data);
static void next_upload(trg_upload *upload);

static void
add_set_common_args(JsonObject * args, gint priority, gchar * dir)
{
    json_object_set_string_member(args, FIELD_FILE_DOWNLOAD_DIR, dir);
    json_object_set_int_member(args, FIELD_BANDWIDTH_PRIORITY,
                               (gint64) priority);
}

void trg_upload_free(trg_upload *upload) {
	g_str_slist_free(upload->list);
	g_free(upload->dir);
	trg_response_free(upload->upload_response);
	g_free(upload);
}

static void next_upload(trg_upload *upload) {
	if (upload->upload_response && upload->progress_index < 1) {
		JsonNode *req = torrent_add_from_response(upload->upload_response, 0);
		/*JsonObject *args =
		if (upload->extra_args)
			add_set_common_args()*/
		upload->progress_index++;
		dispatch_async(upload->client, req, upload_complete_callback, upload);
	} else if (upload->list && upload->progress_index < g_slist_length(upload->list)) {
		JsonNode *req = torrent_add_from_file((gchar*)g_slist_nth_data(upload->list, upload->progress_index), 0);
		upload->progress_index++;
		dispatch_async(upload->client, req, upload_complete_callback, upload);
	} else {
		trg_upload_free(upload);
	}
}

static gboolean upload_complete_callback(gpointer data) {
	trg_response *response = (trg_response*)data;
	trg_upload *upload = (trg_upload*)response->cb_data;

	next_upload(upload);

	/* the callback we're delegating to will destroy the response */

	if (upload->main_window)
		on_generic_interactive_action(upload->main_window, response);
	else /* otherwise need to do it here */
		trg_response_free(response);

	return FALSE;
}

void trg_do_upload(trg_upload *upload)
{
	next_upload(upload);
}
