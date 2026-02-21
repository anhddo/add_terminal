#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IBKR_DLL_EXPORTS
#define IBKR_API __declspec(dllexport)
#else
#define IBKR_API __declspec(dllimport)
#endif

typedef void* IbkrHandle;

/* Event type constants */
#define IBKR_EVENT_NONE            0
#define IBKR_EVENT_HISTORICAL_DATA 1
#define IBKR_EVENT_SCANNER_RESULT  2
#define IBKR_EVENT_ACCOUNT_VALUE   3
#define IBKR_EVENT_POSITION        4

/* Single OHLCV bar */
typedef struct {
    char   date[32];
    double open;
    double high;
    double low;
    double close;
    long   volume;
} IbkrCandle;

/* Payload for IBKR_EVENT_HISTORICAL_DATA */
typedef struct {
    int         reqId;
    char        symbol[32];
    IbkrCandle* candles;     /* heap-allocated; free with ibkr_free_event */
    int         candleCount;
} IbkrHistoricalData;

/* Single scanner result row */
typedef struct {
    int  rank;
    char symbol[32];
    char secType[16];
    char currency[8];
    long conId;
} IbkrScannerItem;

/* Payload for IBKR_EVENT_SCANNER_RESULT */
typedef struct {
    int              reqId;
    IbkrScannerItem* items;  /* heap-allocated; free with ibkr_free_event */
    int              itemCount;
} IbkrScannerResult;

/* Payload for IBKR_EVENT_ACCOUNT_VALUE */
typedef struct {
    char key[64];
    char value[64];
    char currency[8];
    char accountName[32];
} IbkrAccountValue;

/* Payload for IBKR_EVENT_POSITION */
typedef struct {
    char   account[32];
    char   symbol[32];
    char   secType[16];
    double position;
    double marketPrice;
    double marketValue;
    double averageCost;
    double unrealizedPNL;
    double realizedPNL;
} IbkrPosition;

/* Generic event container returned by ibkr_poll_event */
typedef struct {
    int   type; /* IBKR_EVENT_* constant */
    void* data; /* Cast to the matching payload struct; free with ibkr_free_event */
} IbkrCEvent;

/* --- Lifecycle --- */
IBKR_API IbkrHandle ibkr_create(const char* host, int port, int clientId);
IBKR_API void       ibkr_start(IbkrHandle handle);
IBKR_API void       ibkr_destroy(IbkrHandle handle);

/* --- Commands --- */
IBKR_API void ibkr_request_historical_data(
    IbkrHandle handle, int reqId,
    const char* symbol, const char* endDateTime,
    const char* durationStr, const char* barSizeSetting,
    const char* whatToShow, int useRTH);

IBKR_API void ibkr_start_scanner(
    IbkrHandle handle, int reqId,
    const char* scanCode, const char* locationCode, double priceAbove);

IBKR_API void ibkr_cancel_scanner(IbkrHandle handle, int reqId);

IBKR_API void ibkr_request_account_data(IbkrHandle handle, const char* accountCode);

IBKR_API void ibkr_disconnect(IbkrHandle handle);

/* --- Event polling --- */
/* Returns 1 if an event was written to *out, 0 if the queue is empty. */
IBKR_API int  ibkr_poll_event(IbkrHandle handle, IbkrCEvent* out);
/* Frees heap memory owned by an event returned from ibkr_poll_event. */
IBKR_API void ibkr_free_event(IbkrCEvent* event);

#ifdef __cplusplus
}
#endif
