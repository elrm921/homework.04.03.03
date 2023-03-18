#include <iostream>
#include <fstream>
#include <vector>

enum LogMessageType{
    fatal_error,
    error,
    warning
};

class LogMessage {
public:
    LogMessage(const std::string& message, int type) : message_(message), type_(type) {}
    int type() const { return type_; }
    std::string message() const { return message_; }
private:
    int type_;
    std::string message_;
};

class Handler {
public:
    Handler() = default;
    virtual void handle(const LogMessage& message) {
        if (next_ != nullptr) next_->handle(message);
    }
    void append_next(const Handler& next) {
        auto it = this;
        while (it->next_ != nullptr) { it = it->next_; }
        it->next_ = const_cast<Handler*>(&next);
    }
protected:
    Handler* next_ = nullptr;
};

class FatalHandler : public Handler {
public:
    FatalHandler() = default;
    FatalHandler(Handler& next) {
        next_ = &next;
    }
    ~FatalHandler() {
        delete next_;
        next_ = nullptr;
    }
    void handle(const LogMessage& message) override {
        if (message.type() == LogMessageType::fatal_error) {
            throw std::runtime_error(message.message());
        }
        else if (next_ != nullptr) {
            next_->handle(message);
        }
    }
};

class ErrorHandler : public Handler {
public:
    ErrorHandler(const std::string path) {
        path_ = path;
    }
    ErrorHandler(const std::string path, Handler& next) {
        path_ = path;
        next_ = &next;
    }
    ~ErrorHandler() {
        delete next_;
        next_ = nullptr;
    }
    void handle(const LogMessage& message) override {
        if (message.type() == LogMessageType::error) {
            std::ofstream out;
            out.open(path_, std::ios_base::app);
            out << "Error: " << message.message() << std::endl;
            out.close();
        }
        else if (next_ != nullptr) {
            next_->handle(message);
        }
    }
private:
    std::string path_;
};

class WarningHandler : public Handler {
public:
    WarningHandler() = default;
    WarningHandler(Handler& next) {
        next_ = &next;
    }
    ~WarningHandler() {
        delete next_;
        next_ = nullptr;
    }
    void handle(const LogMessage& message) override {
        if (message.type() == LogMessageType::warning) {
            std::cout << "Warning: " << message.message() << std::endl;
        }
        else if (next_ != nullptr) {
            next_->handle(message);
        }
    }
};

class UnknownHandler : public Handler {
public:
    UnknownHandler() = default;
    void handle(const LogMessage& message) override {
        throw std::runtime_error("Unknow error");
    }
};

int main() {

    auto chain = Handler();
    chain.append_next(FatalHandler());
    chain.append_next(ErrorHandler("log.txt"));
    chain.append_next(WarningHandler());
    chain.append_next(UnknownHandler());

    auto fatal_error = LogMessage("fatal message", LogMessageType::fatal_error);
    auto error = LogMessage("error message", LogMessageType::error);
    auto warning = LogMessage("warning message", LogMessageType::warning);
    auto unknown = LogMessage("unknown message", 10);

    for (auto msg : std::vector<LogMessage> {fatal_error, error, warning,  unknown} ) {
        chain.handle(msg);
    }

    return 0;
}