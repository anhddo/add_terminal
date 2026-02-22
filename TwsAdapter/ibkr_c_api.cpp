#include "ibkr_c_api.h"
#include "ibkr.h"
#include "command.h"
#include "event.h"

#include <thread>
#include <chrono>
#include <cstring>
#include <queue>

/* Internal context: owns the IbkrClient and its worker thread */
struct IbkrCtx {
    IbkrClient* client;
    std::thread thread;
};

IbkrHandle ibkr_create(const char* host, int port, int clientId) {
    IbkrCtx* ctx = new IbkrCtx();
    ctx->client = new IbkrClient(host ? host : "127.0.0.1", port, clientId);
    ctx->thread = std::thread([ctx]() {
        ctx->client->processLoop();
    });
    return ctx;
}

void ibkr_destroy(IbkrHandle handle) {
    if (!handle) return;
    IbkrCtx* ctx = static_cast<IbkrCtx*>(handle);
    if (ctx->thread.joinable())
        ctx->thread.join();
    delete ctx->client;
    delete ctx;
}

void ibkr_connect(IbkrHandle /*handle*/) {
    /* processLoop() handles connection internally.
       Sleep to allow the handshake to complete. */
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void ibkr_disconnect(IbkrHandle handle) {
    if (!handle) return;
    IbkrCtx* ctx = static_cast<IbkrCtx*>(handle);
    DiscounnectCommand cmd;
    ctx->client->pushCommand(cmd);
}

void ibkr_start_scanner(IbkrHandle handle, int reqId,
                        const char* scanCode, double priceAbove) {
    if (!handle || !scanCode) return;
    IbkrCtx* ctx = static_cast<IbkrCtx*>(handle);
    StartScannerCommand cmd;
    cmd.reqId      = reqId;
    cmd.scanCode   = scanCode;
    cmd.locationCode = "STK.US";
    cmd.priceAbove = priceAbove;
    ctx->client->pushCommand(std::move(cmd));
}

int ibkr_poll_scanner(IbkrHandle handle,
                      CScannerItem* out_items, int max_items) {
    if (!handle || !out_items || max_items <= 0) return -1;
    IbkrCtx* ctx = static_cast<IbkrCtx*>(handle);

    std::queue<Event> events = ctx->client->consumeEvents();
    while (!events.empty()) {
        if (auto* result = std::get_if<ScannerResult>(&events.front().data)) {
            int count = 0;
            for (const auto& item : result->items) {
                if (count >= max_items) break;
                out_items[count].rank = item.rank;
                strncpy(out_items[count].symbol,   item.symbol.c_str(),
                        sizeof(out_items[count].symbol)   - 1);
                strncpy(out_items[count].secType,  item.secType.c_str(),
                        sizeof(out_items[count].secType)  - 1);
                strncpy(out_items[count].currency, item.currency.c_str(),
                        sizeof(out_items[count].currency) - 1);
                out_items[count].symbol[sizeof(out_items[count].symbol) - 1]     = '\0';
                out_items[count].secType[sizeof(out_items[count].secType) - 1]   = '\0';
                out_items[count].currency[sizeof(out_items[count].currency) - 1] = '\0';
                ++count;
            }
            return count;
        }
        events.pop();
    }
    return -1; /* no ScannerResult in this batch */
}
