#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle - C code never sees the C++ internals */
typedef void* IbkrHandle;

typedef struct {
    int   rank;
    char  symbol[32];
    char  secType[16];
    char  currency[8];
} CScannerItem;

/* Lifecycle */
IbkrHandle ibkr_create(const char* host, int port, int clientId);
void       ibkr_destroy(IbkrHandle handle);
void       ibkr_connect(IbkrHandle handle);
void       ibkr_disconnect(IbkrHandle handle);

/* Scanner */
void ibkr_start_scanner(IbkrHandle handle, int reqId,
                        const char* scanCode, double priceAbove);

/* Poll for scanner results (non-blocking).
   Returns number of items written into out_items (up to max_items).
   Returns -1 if no result is ready yet. */
int  ibkr_poll_scanner(IbkrHandle handle,
                       CScannerItem* out_items, int max_items);

#ifdef __cplusplus
}
#endif
