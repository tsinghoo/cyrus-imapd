/* carddav_db.h -- abstract interface for per-user CardDAV database
 *
 * Copyright (c) 1994-2013 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any legal
 *    details, please contact
 *      Carnegie Mellon University
 *      Center for Technology Transfer and Enterprise Creation
 *      4615 Forbes Avenue
 *      Suite 302
 *      Pittsburgh, PA  15213
 *      (412) 268-7393, fax: (412) 268-7395
 *      innovation@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef CARDDAV_DB_H
#define CARDDAV_DB_H

#include <config.h>

#include "auth.h"
#include "dav_db.h"
#include "strarray.h"
#include "util.h"
#include "vparse.h"

struct carddav_db;

#define CARDDAV_KIND_CONTACT 0
#define CARDDAV_KIND_GROUP 1
struct carddav_data {
    struct dav_data dav;  /* MUST be first so we can typecast */
    unsigned version;
    const char *vcard_uid;
    unsigned kind;
    const char *fullname;
    const char *name;
    const char *nickname;
    strarray_t *emails;
    strarray_t *member_uids;
};

typedef int carddav_cb_t(void *rock, struct carddav_data *cdata);


/* prepare for carddav operations in this process */
int carddav_init(void);

/* done with all carddav operations for this process */
int carddav_done(void);

/* get a database handle corresponding to mailbox */
struct carddav_db *carddav_open_mailbox(struct mailbox *mailbox);
struct carddav_db *carddav_open_userid(const char *userid);

/* close this handle */
int carddav_close(struct carddav_db *carddavdb);

/* lookup an entry from 'carddavdb' by resource
   (optionally inside a transaction for updates) */
int carddav_lookup_resource(struct carddav_db *carddavdb,
                           const char *mailbox, const char *resource,
                           struct carddav_data **result,
                           int tombstones);

/* lookup an entry from 'carddavdb' by iCal UID
   (optionally inside a transaction for updates) */
int carddav_lookup_uid(struct carddav_db *carddavdb, const char *ical_uid,
                       struct carddav_data **result);

/* check if an email address exists on any card.
   returns the groups its in (if any) */
strarray_t *carddav_getemail(struct carddav_db *carddavdb, const char *key);
strarray_t *carddav_getemail2uids(struct carddav_db *carddavdb, const char *key,
                                  const char *mboxname);
strarray_t *carddav_getuid2groups(struct carddav_db *carddavdb, const char *key,
                                  const char *mboxname, const char *otheruser);

/* checks if a group exists (by id).
   returns emails of its members (if any) */
strarray_t *carddav_getgroup(struct carddav_db *carddavdb, const char *mailbox, const char *group);

/* get a list of groups the given uid is a member of */
strarray_t *carddav_getuid_groups(struct carddav_db *carddavdb, const char *uid);

/* process each entry of type 'kind' for 'mailbox' in 'carddavdb' with cb() */
int carddav_get_cards(struct carddav_db *carddavdb,
                      const char *mailbox, const char *vcard_uid, int kind,
                      carddav_cb_t *cb, void *rock);

/* process each entry of type 'kind' and updated since 'oldmodseq'
   in 'carddavdb' with cb() */
int carddav_get_updates(struct carddav_db *carddavdb,
                        modseq_t oldmodseq, int kind,
                        carddav_cb_t *cb, void *rock);

/* process each entry for 'mailbox' in 'carddavdb' with cb() */
int carddav_foreach(struct carddav_db *carddavdb, const char *mailbox,
                    carddav_cb_t *cb, void *rock);

/* write an entry to 'carddavdb' */
int carddav_write(struct carddav_db *carddavdb, struct carddav_data *cdata);

/* write an entry form a vcard */
int carddav_writecard(struct carddav_db *carddavdb, struct carddav_data *cdata,
                      struct vparse_card *vcard);

/* delete an entry from 'carddavdb' */
int carddav_delete(struct carddav_db *carddavdb, unsigned rowid);

/* delete all entries for 'mailbox' from 'carddavdb' */
int carddav_delmbox(struct carddav_db *carddavdb, const char *mailbox);

/* begin transaction */
int carddav_begin(struct carddav_db *carddavdb);

/* commit transaction */
int carddav_commit(struct carddav_db *carddavdb);

/* abort transaction */
int carddav_abort(struct carddav_db *carddavdb);

/* store a vcard to mailbox/resource */
int carddav_store(struct mailbox *mailbox, struct vparse_card *vcard,
                  const char *resource,
                  strarray_t *flags, struct entryattlist *annots,
                  const char *userid, struct auth_state *authstate,
                  int ignorequota);

/* delete a carddav entry */
int carddav_remove(struct mailbox *mailbox,
                   uint32_t olduid, int isreplace);

/* calculate a mailbox name */
char *carddav_mboxname(const char *userid, const char *name);

#endif /* CARDDAV_DB_H */