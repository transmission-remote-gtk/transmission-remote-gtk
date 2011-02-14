/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011  Alan Fitton

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef PROTOCOL_CONSTANTS_H_
#define PROTOCOL_CONSTANTS_H_

/* generic contstants */

#define PARAM_METHOD		"method"
#define FIELD_ID                "id"

/* torrents */

#define FIELD_TORRENTS          "torrents"	/* parent node */
#define FIELD_ANNOUNCE_URL      "announceUrl"
#define FIELD_LEFT_UNTIL_DONE   "leftUntilDone"
#define FIELD_TOTAL_SIZE        "totalSize"
#define FIELD_DONE_DATE         "doneDate"
#define FIELD_ADDED_DATE        "addedDate"
#define FIELD_TRACKERS          "trackers"
#define FIELD_DOWNLOAD_DIR      "downloadDir"
#define FIELD_HASH_STRING       "hashString"
#define FIELD_SWARM_SPEED       "swarmSpeed"
#define FIELD_ERROR_STRING      "errorString"
#define FIELD_NAME				"name"
#define FIELD_SIZEWHENDONE		"sizeWhenDone"
#define FIELD_STATUS			"status"
#define FIELD_MOVE				"move"
#define FIELD_LOCATION			"location"
#define FIELD_RATEDOWNLOAD		"rateDownload"
#define FIELD_RATEUPLOAD		"rateUpload"
#define FIELD_ETA				"eta"
#define FIELD_UPLOADEDEVER		"uploadedEver"
#define FIELD_DOWNLOADEDEVER	"downloadedEver"
#define FIELD_HAVEVALID			"haveValid"
#define FIELD_HAVEUNCHECKED		"haveUnchecked"
#define FIELD_PERCENTDONE		"percentDone"
#define FIELD_TRACKERS          "trackers"
#define FIELD_PEERS             "peers"
#define FIELD_FILES             "files"
#define FIELD_WANTED            "wanted"
#define FIELD_PRIORITIES        "priorities"
#define FIELD_COMMENT           "comment"
#define FIELD_LEFTUNTILDONE     "leftUntilDone"
#define FIELD_ISFINISHED        "isFinished"
#define FIELD_ERRORSTR          "errorString"
#define FIELD_BANDWIDTH_PRIORITY "bandwidthPriority"
#define FIELD_UPLOAD_LIMIT      "uploadLimit"
#define FIELD_UPLOAD_LIMITED    "uploadLimited"
#define FIELD_DOWNLOAD_LIMIT    "downloadLimit"
#define FIELD_DOWNLOAD_LIMITED  "downloadLimited"
#define FIELD_HONORS_SESSION_LIMITS "honorsSessionLimits"
#define FIELD_SEED_RATIO_MODE   "seedRatioMode"
#define FIELD_SEED_RATIO_LIMIT   "seedRatioLimit"
#define FIELD_PEER_LIMIT         "peer-limit"
#define FIELD_DOWNLOAD_DIR		"downloadDir"

#define FIELD_FILES_WANTED      "files-wanted"
#define FIELD_FILES_UNWANTED    "files-unwanted"
#define FIELD_FILES_PRIORITY_HIGH "priority-high"
#define FIELD_FILES_PRIORITY_NORMAL "priority-normal"
#define FIELD_FILES_PRIORITY_LOW "priority-low"

/* trackers */

#define FIELD_TIER              "tier"
#define FIELD_ANNOUNCE          "announce"
#define FIELD_SCRAPE            "scrape"

/* methods */

#define METHOD_TORRENT_START    "torrent-start"
#define METHOD_SESSION_GET      "session-get"
#define METHOD_SESSION_SET      "session-set"
#define METHOD_TORRENT_GET      "torrent-get"
#define METHOD_TORRENT_SET      "torrent-set"
#define METHOD_TORRENT_SET_LOCATION "torrent-set-location"
#define METHOD_TORRENT_STOP     "torrent-stop"
#define METHOD_TORRENT_VERIFY   "torrent-verify"
#define METHOD_TORRENT_REMOVE   "torrent-remove"
#define METHOD_TORRENT_ADD      "torrent-add"
#define METHOD_PORT_TEST		"port-test"
#define METHOD_BLOCKLIST_UPDATE	"blocklist-update"
#define METHOD_SESSION_STATS	"session-stats"

#define PARAM_IDS               "ids"
#define PARAM_DELETE_LOCAL_DATA "delete-local-data"
#define PARAM_ARGUMENTS         "arguments"
#define PARAM_FIELDS            "fields"
#define PARAM_METAINFO          "metainfo"
#define PARAM_FILENAME          "filename"
#define PARAM_TAG               "tag"

enum {
    STATUS_WAITING_TO_CHECK = 1,
    STATUS_CHECKING = 2,
    STATUS_DOWNLOADING = 4,
    STATUS_SEEDING = 8,
    STATUS_PAUSED = 16
} TorrentState;

#define TFILE_LENGTH                            "length"
#define TFILE_BYTES_COMPLETED                   "bytesCompleted"
#define TFILE_NAME                              "name"

#endif				/* PROTOCOL_CONSTANTS_H_ */
