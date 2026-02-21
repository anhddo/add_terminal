/*
 * TwsAdapter\ibkr_c_api.cpp
 *
 * Required additional include directories for this project (in project properties):
 *   ..\chart_glfw\third_party\ibkr_api\client
 *   ..\chart_glfw
 */

#define IBKR_DLL_EXPORTS
#include "ibkr_c_api.h"

#include "../chart_glfw/data_api/ikbr/ibkr.h"

#include <thread>
#include <queue>
#include <cstring>

/* Internal wrapper: holds the C++ client, its background thread, and the
   converted C-event queue that ibkr_poll_event reads from. */
struct IbkrWrapper {
    IbkrClient             client;
    std::thread            processThread;
    std::queue<IbkrCEvent> readyEvents;

    IbkrWrapper(const char* host, int port, int clientId)
        : client(host ? host : "127.0.0.1", port, clientId)
    {}

    ~IbkrWrapper() {
        while (!readyEvents.empty()) {
            IbkrCEvent e = readyEvents.front();
            readyEvents.pop();
            ibkr_free_event(&e);
        }
    }

    /* Drain C++ events from the client and convert them to C structs. */
    void refill() {
        auto events = client.consumeEvents();
        while (!events.empty()) {
            Event ev = std::move(events.front());
            events.pop();
            convertAndPush(ev);
        }
    }

    void convertAndPush(const Event& ev) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, HistoricalDataEvent>) {
                IbkrHistoricalData* data = new IbkrHistoricalData{};
                data->reqId       = arg.reqId;
                data->candleCount = (int)arg.candles.size();
                strncpy_s(data->symbol, sizeof(data->symbol),
                          arg.symbol.c_str(), _TRUNCATE);
                data->candles = new IbkrCandle[data->candleCount];
                for (int i = 0; i < data->candleCount; ++i) {
                    strncpy_s(data->candles[i].date, sizeof(data->candles[i].date),
                              arg.candles[i].date.c_str(), _TRUNCATE);
                    data->candles[i].open   = arg.candles[i].open;
                    data->candles[i].high   = arg.candles[i].high;
                    data->candles[i].low    = arg.candles[i].low;
                    data->candles[i].close  = arg.candles[i].close;
                    data->candles[i].volume = arg.candles[i].volume;
                }
                readyEvents.push({ IBKR_EVENT_HISTORICAL_DATA, data });
            }
            else if constexpr (std::is_same_v<T, ScannerResult>) {
                IbkrScannerResult* data = new IbkrScannerResult{};
                data->reqId     = arg.reqId;
                data->itemCount = (int)arg.items.size();
                data->items = new IbkrScannerItem[data->itemCount];
                for (int i = 0; i < data->itemCount; ++i) {
                    data->items[i].rank  = arg.items[i].rank;
                    data->items[i].conId = arg.items[i].conId;
                    strncpy_s(data->items[i].symbol,   sizeof(data->items[i].symbol),
                              arg.items[i].symbol.c_str(),   _TRUNCATE);
                    strncpy_s(data->items[i].secType,  sizeof(data->items[i].secType),
                              arg.items[i].secType.c_str(),  _TRUNCATE);
                    strncpy_s(data->items[i].currency, sizeof(data->items[i].currency),
                              arg.items[i].currency.c_str(), _TRUNCATE);
                }
                readyEvents.push({ IBKR_EVENT_SCANNER_RESULT, data });
            }
            else if constexpr (std::is_same_v<T, AccountSummaryEvent>) {
                for (auto& [key, val] : arg.accountValues) {
                    IbkrAccountValue* data = new IbkrAccountValue{};
                    strncpy_s(data->key,         sizeof(data->key),
                              val.key.c_str(),         _TRUNCATE);
                    strncpy_s(data->value,        sizeof(data->value),
                              val.value.c_str(),       _TRUNCATE);
                    strncpy_s(data->currency,     sizeof(data->currency),
                              val.currency.c_str(),    _TRUNCATE);
                    strncpy_s(data->accountName,  sizeof(data->accountName),
                              val.accountName.c_str(), _TRUNCATE);
                    readyEvents.push({ IBKR_EVENT_ACCOUNT_VALUE, data });
                }
                for (auto& pos : arg.positions) {
                    IbkrPosition* data = new IbkrPosition{};
                    strncpy_s(data->account, sizeof(data->account),
                              pos.account.c_str(), _TRUNCATE);
                    strncpy_s(data->symbol,  sizeof(data->symbol),
                              pos.symbol.c_str(),  _TRUNCATE);
                    strncpy_s(data->secType, sizeof(data->secType),
                              pos.secType.c_str(), _TRUNCATE);
                    data->position      = pos.position;
                    data->marketPrice   = pos.marketPrice;
                    data->marketValue   = pos.marketValue;
                    data->averageCost   = pos.averageCost;
                    data->unrealizedPNL = pos.unrealizedPNL;
                    data->realizedPNL   = pos.realizedPNL;
                    readyEvents.push({ IBKR_EVENT_POSITION, data });
                }
            }
        }, ev.data);
    }
};

extern "C" {

IbkrHandle ibkr_create(const char* host, int port, int clientId) {
    return new IbkrWrapper(host, port, clientId);
}

void ibkr_start(IbkrHandle handle) {
    auto* w = static_cast<IbkrWrapper*>(handle);
    w->processThread = std::thread([w]() { w->client.processLoop(); });
}

void ibkr_destroy(IbkrHandle handle) {
    auto* w = static_cast<IbkrWrapper*>(handle);
    w->client.pushCommand(DiscounnectCommand{});
    if (w->processThread.joinable())
        w->processThread.join();
    delete w;
}

void ibkr_request_historical_data(IbkrHandle handle, int reqId,
    const char* symbol, const char* endDateTime,
    const char* durationStr, const char* barSizeSetting,
    const char* whatToShow, int useRTH)
{
    auto* w = static_cast<IbkrWrapper*>(handle);
    RequestHistoricalDataCommand cmd;
    cmd.reqId          = reqId;
    cmd.symbol         = symbol         ? symbol         : "";
    cmd.endDateTime    = endDateTime    ? endDateTime    : "";
    cmd.durationStr    = durationStr    ? durationStr    : "1 M";
    cmd.barSizeSetting = barSizeSetting ? barSizeSetting : "1 day";
    cmd.whatToShow     = whatToShow     ? whatToShow     : "MIDPOINT";
    cmd.useRTH         = useRTH;
    w->client.pushCommand(std::move(cmd));
}

void ibkr_start_scanner(IbkrHandle handle, int reqId,
    const char* scanCode, const char* locationCode, double priceAbove)
{
    auto* w = static_cast<IbkrWrapper*>(handle);
    StartScannerCommand cmd;
    cmd.reqId        = reqId;
    cmd.scanCode     = scanCode     ? scanCode     : "";
    cmd.locationCode = locationCode ? locationCode : "STK.US";
    cmd.priceAbove   = priceAbove;
    w->client.pushCommand(std::move(cmd));
}

void ibkr_cancel_scanner(IbkrHandle handle, int reqId) {
    auto* w = static_cast<IbkrWrapper*>(handle);
    w->client.pushCommand(CancelScannerCommand{ reqId });
}

void ibkr_request_account_data(IbkrHandle handle, const char* accountCode) {
    auto* w = static_cast<IbkrWrapper*>(handle);
    RequestAccountDataCommand cmd;
    cmd.accountCode = accountCode ? accountCode : "";
    w->client.pushCommand(std::move(cmd));
}

void ibkr_disconnect(IbkrHandle handle) {
    auto* w = static_cast<IbkrWrapper*>(handle);
    w->client.pushCommand(DiscounnectCommand{});
}

int ibkr_poll_event(IbkrHandle handle, IbkrCEvent* out) {
    auto* w = static_cast<IbkrWrapper*>(handle);
    if (w->readyEvents.empty())
        w->refill();
    if (w->readyEvents.empty()) {
        out->type = IBKR_EVENT_NONE;
        out->data = NULL;
        return 0;
    }
    *out = w->readyEvents.front();
    w->readyEvents.pop();
    return 1;
}

void ibkr_free_event(IbkrCEvent* event) {
    if (!event || !event->data) return;
    switch (event->type) {
    case IBKR_EVENT_HISTORICAL_DATA: {
        IbkrHistoricalData* d = (IbkrHistoricalData*)event->data;
        delete[] d->candles;
        delete d;
        break;
    }
    case IBKR_EVENT_SCANNER_RESULT: {
        IbkrScannerResult* d = (IbkrScannerResult*)event->data;
        delete[] d->items;
        delete d;
        break;
    }
    case IBKR_EVENT_ACCOUNT_VALUE:
        delete (IbkrAccountValue*)event->data;
        break;
    case IBKR_EVENT_POSITION:
        delete (IbkrPosition*)event->data;
        break;
    }
    event->data = NULL;
    event->type = IBKR_EVENT_NONE;
}

} /* extern "C" */
