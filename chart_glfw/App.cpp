#include "App.h"
#include "data_api/ikbr/ibkr.h"
#include <chrono>
#include <iostream>
#include "renderer.h"
#include "command.h"

App::App() 
    : m_scannerReqId(0)
{
}

App::~App() 
{
    stop();
}

void App::init(GLFWwindow* window)
{
    m_ibClient = std::make_unique<IbkrClient>();
    m_ibThread = std::thread([this]() {
        m_ibClient->processLoop();
    });
    
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    startScanner(1, "TOP_PERC_GAIN");
	m_renderer = std::make_unique<Renderer>();

	m_renderer->init(window);

	// Set window user pointer so scroll callback can access renderer
	glfwSetWindowUserPointer(window, m_renderer.get());

	// Wire up symbol input callback
	m_renderer->onSymbolEntered = [this](const std::string& symbol) {
		if (dataManager.charts.find(symbol) != dataManager.charts.end()) {
            dataManager.activeSymbol = symbol;
            printf("Switched active chart to %s\n", symbol.c_str());
        } else {
            printf("No chart data for %s, requesting...\n", symbol.c_str());
            requestChart(symbol);
        }
	};

    m_renderer->onScannerRowClicked = [this](const std::string& symbol) {
        if (dataManager.charts.find(symbol) != dataManager.charts.end()) {
            dataManager.activeSymbol = symbol;
            printf("Switched active chart to %s\n", symbol.c_str());
        }
        else {
            printf("No chart data for %s, requesting...\n", symbol.c_str());
            requestChart(symbol);
        }
	};

}

void App::stop()
{
    DiscounnectCommand disconnectCmd;

    m_ibClient->pushCommand(disconnectCmd);
    // 3. Wait for thread to finish
    if (m_ibThread.joinable()) {
        printf("Waiting for IB thread to finish...\n");
        m_ibThread.join();
        printf("IB thread joined\n");
    }

    //printf("App::stop() finished\n");
    //if (m_ibClient)
    //    m_ibClient->stop();
}

void App::update()
{
    std::queue<Event> eventQueue = m_ibClient->consumeEvents();

    while (!eventQueue.empty())
    {
        handleEvent(eventQueue.front());
        eventQueue.pop();
    }
    m_renderer->draw(dataManager);
}

void App::startScanner(int reqId, const std::string& scanCode, double priceAbove)
{
    StartScannerCommand command;
    command.reqId = reqId;
    command.scanCode = scanCode;
    command.locationCode = "STK.US";
    command.priceAbove = priceAbove;

    
    m_ibClient->pushCommand(std::move(command));

    printf("UI: Scanner command sent (reqId=%d, scanCode=%s)\n", reqId, scanCode.c_str());
}

void App::handleEvent(const Event& event)
{
    std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, ScannerResult>) {
            dataManager.currentScannerResult = arg;

            CancelScannerCommand cancelCmd;
            cancelCmd.reqId = arg.reqId;
            
            m_ibClient->pushCommand(std::move(cancelCmd));
        }
        else if constexpr (std::is_same_v<T, HistoricalDataEvent>) {
            // Store chart data
            ChartData chartData;
            chartData.symbol = arg.symbol;
            chartData.candles = arg.candles;
            chartData.reqId = arg.reqId;

            dataManager.charts[arg.symbol] = chartData;
            dataManager.activeSymbol = arg.symbol;

            printf("Chart data received for %s: %zu candles\n", 
                   arg.symbol.c_str(), arg.candles.size());
        }
    }, event.data);
}

void App::requestChart(const std::string& symbol)
{
    RequestHistoricalDataCommand cmd;
    cmd.reqId = m_nextReqId++;
    cmd.symbol = symbol;
    cmd.endDateTime = "";           // Empty = now
    cmd.durationStr = "1 Y";        // 1 year of data (maximum for daily bars)
    cmd.barSizeSetting = "1 day";   // Daily candles
    cmd.whatToShow = "TRADES";
    cmd.useRTH = 1;                 // Regular trading hours only

    
    m_ibClient->pushCommand(std::move(cmd));

    printf("Requesting daily chart for %s (reqId=%d, duration=%s)\n", 
           symbol.c_str(), cmd.reqId, cmd.durationStr.c_str());
}
