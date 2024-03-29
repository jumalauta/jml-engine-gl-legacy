#include "device.h"
#include "track.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

//MR 2015/11 - sync editor & player must work dynamically without recompilation
#include "synceditor.h"
#include "system/debug/debug.h"
#include "system/xml/xml.h"

static int find_track(struct sync_device *d, const char *name)
{
	int i;
	for (i = 0; i < (int)d->num_tracks; ++i)
		if (!strcmp(name, d->tracks[i]->name))
			return i;
	debugPrintf("Did not find '%s'", name);
	return -1; /* not found */
}

static const char *sync_track_path(const char *base, const char *name)
{
	static char temp[FILENAME_MAX];
	strncpy(temp, base, sizeof(temp) - 1);
	temp[sizeof(temp) - 1] = '\0';
	strncat(temp, "_", sizeof(temp) - 1);
	strncat(temp, name, sizeof(temp) - 1);
	strncat(temp, ".track", sizeof(temp) - 1);
	return temp;
}

//#ifndef SYNC_PLAYER

#define CLIENT_GREET "hello, synctracker!"
#define SERVER_GREET "hello, demo!"

enum {
	SET_KEY = 0,
	DELETE_KEY = 1,
	GET_TRACK = 2,
	SET_ROW = 3,
	PAUSE = 4,
	SAVE_TRACKS = 5
};

static inline int socket_poll(SOCKET socket)
{
	struct timeval to = { 0, 0 };
	fd_set fds;

	FD_ZERO(&fds);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
	FD_SET(socket, &fds);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	return select((int)socket + 1, &fds, NULL, NULL, &to) > 0;
}

static inline int xsend(SOCKET s, const void *buf, size_t len, int flags)
{
#ifdef WIN32
	assert(len <= INT_MAX);
	return send(s, (const char *)buf, (int)len, flags) != (int)len;
#else
	return send(s, (const char *)buf, len, flags) != (int)len;
#endif
}

static inline int xrecv(SOCKET s, void *buf, size_t len, int flags)
{
#ifdef WIN32
	assert(len <= INT_MAX);
	return recv(s, (char *)buf, (int)len, flags) != (int)len;
#else
	return recv(s, (char *)buf, len, flags) != (int)len;
#endif
}

#ifdef USE_AMITCP
static struct Library *socket_base = NULL;
#endif

static SOCKET server_connect(const char *host, unsigned short nport)
{
#ifdef USE_GETADDRINFO
	struct addrinfo *addr;
	char port[6];
#else
	struct hostent *he;
	char **ap;
#endif

#ifdef WIN32
	static int need_init = 1;
	if (need_init) {
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 0), &wsa))
			return INVALID_SOCKET;
		need_init = 0;
	}
#elif defined(USE_AMITCP)
	if (!socket_base) {
		socket_base = OpenLibrary("bsdsocket.library", 4);
		if (!socket_base)
			return INVALID_SOCKET;
	}
#endif

#ifdef USE_GETADDRINFO

	snprintf(port, sizeof(port), "%u", nport);
	if (getaddrinfo(host, port, 0, &addr) != 0)
		return INVALID_SOCKET;

	for (; addr; addr = addr->ai_next) {
		SOCKET sock;
		int family = addr->ai_family;
		struct sockaddr *sa = addr->ai_addr;
		int sa_len = (int) addr->ai_addrlen; /* elim. warning on (at least) Win/x64, size_t vs. int/socklen_t */

#else

	he = gethostbyname(host);
	if (!he)
		return INVALID_SOCKET;

	for (ap = he->h_addr_list; *ap; ++ap) {
		SOCKET sock;
		int family = he->h_addrtype;
		struct sockaddr_in sin;
		struct sockaddr *sa = (struct sockaddr *)&sin;
		int sa_len = sizeof(*sa);

		sin.sin_family = he->h_addrtype;
		sin.sin_port = htons(nport);
		memcpy(&sin.sin_addr, *ap, he->h_length);
		memset(&sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

		sock = socket(family, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET)
			continue;

		if (connect(sock, sa, sa_len) >= 0) {
			char greet[128];

			if (xsend(sock, CLIENT_GREET, strlen(CLIENT_GREET), 0) ||
				xrecv(sock, greet, strlen(SERVER_GREET), 0)) {
				closesocket(sock);
				continue;
			}

			if (!strncmp(SERVER_GREET, greet, strlen(SERVER_GREET)))
				return sock;
		}

		closesocket(sock);
	}

	return INVALID_SOCKET;
}

//#else

void sync_set_io_cb(struct sync_device *d, struct sync_io_cb *cb)
{
	d->io_cb.open = cb->open;
	d->io_cb.read = cb->read;
	d->io_cb.close = cb->close;
}

//#endif

#ifdef NEED_STRDUP
static inline char *rocket_strdup(const char *str)
{
	char *ret = malloc(strlen(str) + 1);
	if (ret)
		strcpy(ret, str);
	return ret;
}
#define strdup rocket_strdup
#endif

struct sync_device *sync_create_device(const char *base)
{
	struct sync_device *d = malloc(sizeof(*d));
	if (!d)
		return NULL;

	d->base = strdup(base);
	if (!d->base) {
		free(d);
		return NULL;
	}

	d->tracks = NULL;
	d->num_tracks = 0;

//MR 2015/11 - sync editor & player must work dynamically without recompilation
//#ifndef SYNC_PLAYER
	d->row = -1;
	d->sock = INVALID_SOCKET;
//#else
	d->io_cb.open = (void *(*)(const char *, const char *))fopen;
	d->io_cb.read = (size_t (*)(void *, size_t, size_t, void *))fread;
	d->io_cb.close = (int (*)(void *))fclose;
//#endif

	return d;
}

void sync_destroy_device(struct sync_device *d)
{
	int i;
	for (i = 0; i < (int)d->num_tracks; ++i) {
		free(d->tracks[i]->name);
		free(d->tracks[i]->keys);
		free(d->tracks[i]);
	}
	free(d->tracks);
	free(d->base);
	free(d);

#if defined(USE_AMITCP)
	if (socket_base && !isSyncEditor()) {
		CloseLibrary(socket_base);
		socket_base = NULL;
	}
#endif
}

static int get_track_data_editor(struct sync_device *d, struct sync_track *t)
{
	unsigned char cmd = GET_TRACK;
	uint32_t name_len;

	assert(strlen(t->name) <= UINT32_MAX);
	name_len = htonl((uint32_t)strlen(t->name));

	/* send request data */
	if (xsend(d->sock, (char *)&cmd, 1, 0) ||
	    xsend(d->sock, (char *)&name_len, sizeof(name_len), 0) ||
	    xsend(d->sock, t->name, (int)strlen(t->name), 0))
	{
		closesocket(d->sock);
		d->sock = INVALID_SOCKET;
		return -1;
	}

	return 0;
}

static void readLittleEndianInt32(void *fp, struct sync_device *d,int *n)
{
   assert(d->io_cb.read(n, 4, 1, fp) == 1);

#ifdef ENGINE_BIG_ENDIAN
   unsigned char *cptr,tmp;
   cptr = (unsigned char *)n;
   tmp = cptr[0];
   cptr[0] = cptr[3];
   cptr[3] = tmp;
   tmp = cptr[1];
   cptr[1] = cptr[2];
   cptr[2] = tmp;
#endif
}

static void readLittleEndianFloat32(void *fp, struct sync_device *d,float *n)
{
   assert(d->io_cb.read(n, 4, 1, fp) == 1);

#ifdef ENGINE_BIG_ENDIAN
   unsigned char *cptr,tmp;
   cptr = (unsigned char *)n;
   tmp = cptr[0];
   cptr[0] = cptr[3];
   cptr[3] =tmp;
   tmp = cptr[1];
   cptr[1] = cptr[2];
   cptr[2] = tmp;
#endif
}

static int get_track_data_player(struct sync_device *d, struct sync_track *t)
{
	int i;
	const char *filename = sync_track_path(d->base, t->name);
	void *fp = d->io_cb.open(filename, "rb");
	if (!fp) {
		debugPrintf("Could not read track '%s'", filename);
		return -1;
	}

	readLittleEndianInt32(fp, d, &t->num_keys);
	t->keys = malloc(sizeof(struct track_key) * t->num_keys);
	if (!t->keys) {
		debugPrintf("Track has no keys '%s'", filename);
		return -1;
	}

	debugPrintf("Reading track '%s' (keys: %d)", filename, (int)t->num_keys);

	for (i = 0; i < (int)t->num_keys; ++i) {
		struct track_key *key = t->keys + i;
		char type;
		//d->io_cb.read(&key->row, sizeof(int), 1, fp);
		//d->io_cb.read(&key->value, sizeof(float), 1, fp);
		readLittleEndianInt32(fp, d, &key->row);
		readLittleEndianFloat32(fp, d, &key->value);
		d->io_cb.read(&type, sizeof(char), 1, fp);
		key->type = (enum key_type)type;
	}

	d->io_cb.close(fp);
	return 0;
}
static int get_track_data(struct sync_device *d, struct sync_track *t)
{
	if(isSyncEditor())
		return get_track_data_editor(d, t);
	else
		return get_track_data_player(d, t);
}

//#ifdef SYNC_PLAYER



//#else

static int save_track(const struct sync_track *t, const char *path)
{
	int i;
	FILE *fp = fopen(path, "wb");
	if (!fp)
		return -1;

	fwrite(&t->num_keys, sizeof(int), 1, fp);
	for (i = 0; i < (int)t->num_keys; ++i) {
		char type = (char)t->keys[i].type;
		fwrite(&t->keys[i].row, sizeof(int), 1, fp);
		fwrite(&t->keys[i].value, sizeof(float), 1, fp);
		fwrite(&type, sizeof(char), 1, fp);
	}

	fclose(fp);
	return 0;
}

void sync_save_tracks(const struct sync_device *d)
{
	int i;
	for (i = 0; i < (int)d->num_tracks; ++i) {
		const struct sync_track *t = d->tracks[i];
		save_track(t, sync_track_path(d->base, t->name));
	}
}

static int handle_set_key_cmd(SOCKET sock, struct sync_device *data)
{
	uint32_t track, row;
	union {
		float f;
		uint32_t i;
	} v;
	struct track_key key;
	unsigned char type;

	if (xrecv(sock, (char *)&track, sizeof(track), 0) ||
	    xrecv(sock, (char *)&row, sizeof(row), 0) ||
	    xrecv(sock, (char *)&v.i, sizeof(v.i), 0) ||
	    xrecv(sock, (char *)&type, 1, 0))
		return -1;

	track = ntohl(track);
	v.i = ntohl(v.i);

	key.row = ntohl(row);
	key.value = v.f;

	assert(type < KEY_TYPE_COUNT);
	assert(track < data->num_tracks);
	key.type = (enum key_type)type;
	return sync_set_key(data->tracks[track], &key);
}

static int handle_del_key_cmd(SOCKET sock, struct sync_device *data)
{
	uint32_t track, row;

	if (xrecv(sock, (char *)&track, sizeof(track), 0) ||
	    xrecv(sock, (char *)&row, sizeof(row), 0))
		return -1;

	track = ntohl(track);
	row = ntohl(row);

	assert(track < data->num_tracks);
	return sync_del_key(data->tracks[track], row);
}

int sync_connect(struct sync_device *d, const char *host, unsigned short port)
{
	int i;
	if (d->sock != INVALID_SOCKET)
		closesocket(d->sock);

	d->sock = server_connect(host, port);
	if (d->sock == INVALID_SOCKET)
		return -1;

	for (i = 0; i < (int)d->num_tracks; ++i) {
		free(d->tracks[i]->keys);
		d->tracks[i]->keys = NULL;
		d->tracks[i]->num_keys = 0;
	}

	for (i = 0; i < (int)d->num_tracks; ++i) {
		if (get_track_data(d, d->tracks[i])) {
			closesocket(d->sock);
			d->sock = INVALID_SOCKET;
			return -1;
		}
	}

	return 0;
}

int sync_update(struct sync_device *d, int row, struct sync_cb *cb,
    void *cb_param)
{
	if (d->sock == INVALID_SOCKET)
		return -1;

	/* look for new commands */
	while (socket_poll(d->sock)) {
		unsigned char cmd = 0, flag;
		uint32_t row;
		if (xrecv(d->sock, (char *)&cmd, 1, 0))
			goto sockerr;

		switch (cmd) {
		case SET_KEY:
			if (handle_set_key_cmd(d->sock, d))
				goto sockerr;
			break;
		case DELETE_KEY:
			if (handle_del_key_cmd(d->sock, d))
				goto sockerr;
			break;
		case SET_ROW:
			if (xrecv(d->sock, (char *)&row, sizeof(row), 0))
				goto sockerr;
			if (cb && cb->set_row)
				cb->set_row(cb_param, ntohl(row));
			break;
		case PAUSE:
			if (xrecv(d->sock, (char *)&flag, 1, 0))
				goto sockerr;
			if (cb && cb->pause)
				cb->pause(cb_param, flag);
			break;
		case SAVE_TRACKS:
			sync_save_tracks(d);
			break;
		default:
			fprintf(stderr, "unknown cmd: %02x\n", cmd);
			goto sockerr;
		}
	}

	if (cb && cb->is_playing && cb->is_playing(cb_param)) {
		if (d->row != row && d->sock != INVALID_SOCKET) {
			unsigned char cmd = SET_ROW;
			uint32_t nrow = htonl(row);
			if (xsend(d->sock, (char*)&cmd, 1, 0) ||
			    xsend(d->sock, (char*)&nrow, sizeof(nrow), 0))
				goto sockerr;
			d->row = row;
		}
	}
	return 0;

sockerr:
	closesocket(d->sock);
	d->sock = INVALID_SOCKET;
	return -1;
}

//#endif

static int create_track(struct sync_device *d, const char *name)
{
	struct sync_track *t;
	assert(find_track(d, name) < 0);

	t = malloc(sizeof(*t));
	t->name = strdup(name);
	t->keys = NULL;
	t->num_keys = 0;

	d->num_tracks++;
	d->tracks = realloc(d->tracks, sizeof(d->tracks[0]) * d->num_tracks);
	d->tracks[d->num_tracks - 1] = t;

	return (int)d->num_tracks - 1;
}

const struct sync_track *sync_get_track(struct sync_device *d,
    const char *name)
{
	struct sync_track *t;
	int idx = find_track(d, name);
	if (idx >= 0)
		return d->tracks[idx];

	idx = create_track(d, name);
	t = d->tracks[idx];

	get_track_data(d, t);
	return t;
}

void parseRocketXml(struct sync_device *d, const char *filename)
{
	xml_t xml;
	if (!xmlInit(&xml)) {
		debugErrorPrintf("Could not init xml");
		return;
	}
	
	xmlSetParseFile(&xml, filename);

	if (!xmlParse(&xml)) {
		debugErrorPrintf("Could not parse xml");
		return;
	}

	xml_element_t *sync = xml.rootElement;
	assert(!strcmp(sync->name, "sync"));

	//const char *rows = elementGetAttributeValue(sync, "rows");
	//printf("sync rows: %s\n", rows);

	xml_element_t *tracks = sync->elements[0];
	assert(!strcmp(tracks->name, "tracks"));
	int i;
	for(i = 0; i < tracks->elementsSize; i++)
	{
		xml_element_t *track = tracks->elements[i];
		if (strcmp(track->name, "track")) {
			continue;
		}

		const char* trackName = elementGetAttributeValue(track, "name");

		int idx = find_track(d, trackName);
		if (idx >= 0 || track->elementsSize == 0) {
			continue;
		}

		idx = create_track(d, trackName);
		struct sync_track *t = d->tracks[idx];

		t->num_keys = track->elementsSize;
		t->keys = malloc(sizeof(struct track_key) * t->num_keys);
		if (!t->keys) {
			debugErrorPrintf("Could not allocate memory. track:'%s', keys:%d", trackName, t->num_keys);
			continue;
		}

		debugPrintf("Processing track '%s' (keys: %d)", trackName, t->num_keys);
		int j;
		for(j = 0; j < track->elementsSize; j++)
		{
			xml_element_t *key = track->elements[j];
			if (strcmp(key->name, "key")) {
				continue;
			}

			struct track_key *k = t->keys + j;
			k->value = atof(elementGetAttributeValue(key, "value"));
			k->row = atoi(elementGetAttributeValue(key, "row"));
			k->type = atoi(elementGetAttributeValue(key, "interpolation"));
		}
	}

	xmlDeinit(&xml);
}
