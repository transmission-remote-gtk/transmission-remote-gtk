/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011-2013  Alan Fitton

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

#define PARAM_METHOD "method"
#define FIELD_ID     "id"

/* top level */

#define FIELD_RESULT  "result"
#define FIELD_SUCCESS "success"

/* torrents */

#define FIELD_RECENTLY_ACTIVE         "recently-active"
#define FIELD_TORRENTS                "torrents" /* parent node */
#define FIELD_REMOVED                 "removed"
#define FIELD_ANNOUNCE_URL            "announceUrl"
#define FIELD_LEFT_UNTIL_DONE         "leftUntilDone"
#define FIELD_TOTAL_SIZE              "totalSize"
#define FIELD_DONE_DATE               "doneDate"
#define FIELD_ADDED_DATE              "addedDate"
#define FIELD_DATE_CREATED            "dateCreated"
#define FIELD_TRACKER_STATS           "trackerStats"
#define FIELD_DOWNLOAD_DIR            "downloadDir"
#define FIELD_HASH_STRING             "hashString"
#define FIELD_NAME                    "name"
#define FIELD_PATH                    "path"
#define FIELD_SIZEWHENDONE            "sizeWhenDone"
#define FIELD_STATUS                  "status"
#define FIELD_MOVE                    "move"
#define FIELD_CREATOR                 "creator"
#define FIELD_LOCATION                "location"
#define FIELD_RATEDOWNLOAD            "rateDownload"
#define FIELD_RATEUPLOAD              "rateUpload"
#define FIELD_ETA                     "eta"
#define FIELD_UPLOADEDEVER            "uploadedEver"
#define FIELD_DOWNLOADEDEVER          "downloadedEver"
#define FIELD_CORRUPTEVER             "corruptEver"
#define FIELD_HAVEVALID               "haveValid"
#define FIELD_HAVEUNCHECKED           "haveUnchecked"
#define FIELD_PERCENTDONE             "percentDone"
#define FIELD_PEERS                   "peers"
#define FIELD_PEERSFROM               "peersFrom"
#define FIELD_FILES                   "files"
#define FIELD_WANTED                  "wanted"
#define FIELD_WEB_SEEDS_SENDING_TO_US "webseedsSendingToUs"
#define FIELD_PRIORITIES              "priorities"
#define FIELD_COMMENT                 "comment"
#define FIELD_LEFTUNTILDONE           "leftUntilDone"
#define FIELD_ISFINISHED              "isFinished"
#define FIELD_ISPRIVATE               "isPrivate"
#define FIELD_MAGNETLINK              "magnetLink"
#define FIELD_ERROR                   "error"
#define FIELD_ERROR_STRING            "errorString"
#define FIELD_BANDWIDTH_PRIORITY      "bandwidthPriority"
#define FIELD_UPLOAD_LIMIT            "uploadLimit"
#define FIELD_UPLOAD_LIMITED          "uploadLimited"
#define FIELD_DOWNLOAD_LIMIT          "downloadLimit"
#define FIELD_DOWNLOAD_LIMITED        "downloadLimited"
#define FIELD_HONORS_SESSION_LIMITS   "honorsSessionLimits"
#define FIELD_SEED_RATIO_MODE         "seedRatioMode"
#define FIELD_SEED_RATIO_LIMIT        "seedRatioLimit"
#define FIELD_PEER_LIMIT              "peer-limit"
#define FIELD_DOWNLOAD_DIR            "downloadDir"
#define FIELD_FILE_DOWNLOAD_DIR       "download-dir"
#define FIELD_PEERS_SENDING_TO_US     "peersSendingToUs"
#define FIELD_PEERS_GETTING_FROM_US   "peersGettingFromUs"
#define FIELD_PEERS_CONNECTED         "peersConnected"
#define FIELD_QUEUE_POSITION          "queuePosition"
#define FIELD_ACTIVITY_DATE           "activityDate"
#define FIELD_ISPRIVATE               "isPrivate"
#define FIELD_METADATAPERCENTCOMPLETE "metadataPercentComplete"
#define FIELD_FILES_WANTED            "files-wanted"
#define FIELD_FILES_UNWANTED          "files-unwanted"
#define FIELD_FILES_PRIORITY_HIGH     "priority-high"
#define FIELD_FILES_PRIORITY_NORMAL   "priority-normal"
#define FIELD_FILES_PRIORITY_LOW      "priority-low"

/* trackers */

#define FIELD_TIER                     "tier"
#define FIELD_ANNOUNCE                 "announce"
#define FIELD_SCRAPE                   "scrape"
#define FIELD_LAST_ANNOUNCE_PEER_COUNT "lastAnnouncePeerCount"
#define FIELD_LAST_ANNOUNCE_TIME       "lastAnnounceTime"
#define FIELD_LAST_SCRAPE_TIME         "lastScrapeTime"
#define FIELD_SEEDERCOUNT              "seederCount"
#define FIELD_LEECHERCOUNT             "leecherCount"
#define FIELD_DOWNLOADCOUNT            "downloadCount"
#define FIELD_HOST                     "host"
#define FIELD_LAST_ANNOUNCE_RESULT     "lastAnnounceResult"
#define FIELD_RECHECK_PROGRESS         "recheckProgress"

/* methods */

#define METHOD_TORRENT_START        "torrent-start"
#define METHOD_SESSION_GET          "session-get"
#define METHOD_SESSION_SET          "session-set"
#define METHOD_TORRENT_GET          "torrent-get"
#define METHOD_TORRENT_SET          "torrent-set"
#define METHOD_TORRENT_SET_LOCATION "torrent-set-location"
#define METHOD_TORRENT_RENAME_PATH  "torrent-rename-path"
#define METHOD_TORRENT_STOP         "torrent-stop"
#define METHOD_TORRENT_VERIFY       "torrent-verify"
#define METHOD_TORRENT_REMOVE       "torrent-remove"
#define METHOD_TORRENT_ADD          "torrent-add"
#define METHOD_TORRENT_REANNOUNCE   "torrent-reannounce"
#define METHOD_PORT_TEST            "port-test"
#define METHOD_BLOCKLIST_UPDATE     "blocklist-update"
#define METHOD_SESSION_STATS        "session-stats"
#define METHOD_QUEUE_MOVE_TOP       "queue-move-top"
#define METHOD_QUEUE_MOVE_UP        "queue-move-up"
#define METHOD_QUEUE_MOVE_BOTTOM    "queue-move-bottom"
#define METHOD_QUEUE_MOVE_DOWN      "queue-move-down"
#define METHOD_TORRENT_START_NOW    "torrent-start-now"

#define PARAM_IDS               "ids"
#define PARAM_DELETE_LOCAL_DATA "delete-local-data"
#define PARAM_ARGUMENTS         "arguments"
#define PARAM_FIELDS            "fields"
#define PARAM_METAINFO          "metainfo"
#define PARAM_FILENAME          "filename"
#define PARAM_PAUSED            "paused"
#define PARAM_TAG               "tag"

/* peers structure */

#define TPEER_ADDRESS             "address"
#define TPEER_CLIENT_NAME         "clientName"
#define TPEER_PROGRESS            "progress"
#define TPEER_RATE_TO_CLIENT      "rateToClient"
#define TPEER_RATE_TO_PEER        "rateToPeer"
#define TPEER_IS_ENCRYPTED        "isEncrypted"
#define TPEER_IS_DOWNLOADING_FROM "isDownloadingFrom"
#define TPEER_IS_UPLOADING_TO     "isUploadingTo"
#define TPEER_FLAGSTR             "flagStr"

#define TPEERFROM_FROMPEX      "fromPex"
#define TPEERFROM_FROMDHT      "fromDht"
#define TPEERFROM_FROMTRACKERS "fromTracker"
#define TPEERFROM_FROMLTEP     "fromLtep"
#define TPEERFROM_FROMRESUME   "fromCache"
#define TPEERFROM_FROMINCOMING "fromIncoming"
#define TPEERFROM_FROMLPD      "fromLpd"

/* The rpc-version >= that the status field of torrent-get changed */
#define NEW_STATUS_RPC_VERSION 14

typedef enum {
    OLD_STATUS_WAITING_TO_CHECK = 1,
    OLD_STATUS_CHECKING = 2,
    OLD_STATUS_DOWNLOADING = 4,
    OLD_STATUS_SEEDING = 8,
    OLD_STATUS_PAUSED = 16
} trg_old_status;

typedef enum {
    TR_STATUS_STOPPED = 0, /* Torrent is stopped */
    TR_STATUS_CHECK_WAIT = 1, /* Queued to check files */
    TR_STATUS_CHECK = 2, /* Checking files */
    TR_STATUS_DOWNLOAD_WAIT = 3, /* Queued to download */
    TR_STATUS_DOWNLOAD = 4, /* Downloading */
    TR_STATUS_SEED_WAIT = 5, /* Queued to seed */
    TR_STATUS_SEED = 6 /* Seeding */
} tr_torrent_activity;

enum {
    TR_PRI_UNSET = -3, /* Not actually in the protocol. Just used in UI. */
    TR_PRI_MIXED = -2, /* Neither is this. */
    TR_PRI_LOW = -1,
    TR_PRI_NORMAL = 0, /* since NORMAL is 0, memset initializes nicely */
    TR_PRI_HIGH = 1
};

#define TFILE_LENGTH          "length"
#define TFILE_BYTES_COMPLETED "bytesCompleted"
#define TFILE_NAME            "name"

#endif /* PROTOCOL_CONSTANTS_H_ */
