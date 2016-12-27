#ifndef logger_h
#define logger_h

using namespace std;

enum class LogLevel {
  Disabled = 0,
  Error = 1,
  Warning = 2,
  Info = 3,
  Debug = 4,
  Verbose = 5,
};

class LOG {
public:
    LOG() {}
    LOG(LogLevel type, string msg) {
        msgLevel = type;
        if(msgLevel <= GlobalLogLevel) {
            operator << ("[" + getLabel(type) + "] " + msg);
            opened = true;
        }
    }
    ~LOG() {
        if(opened) {
            cout << endl;
        }
        opened = false;
    }
    template<class T>
    LOG &operator<<(const T &msg) {
        switch(msgLevel) {
            case LogLevel::Error:
                cerr << msg;
                break;
            default:
                cout << msg;
        }
        opened = true;
        return *this;
    }
private:
    // Change this log level to get different log messages.
    // You can also disable with `LogLevel::Disable`
    static const LogLevel GlobalLogLevel = LogLevel::Info;

    bool opened = false;
    LogLevel msgLevel = LogLevel::Disabled;
    inline string getLabel(LogLevel type) {
        switch(type) {
            case LogLevel::Disabled:
                return "DISABLED";
            case LogLevel::Error:
                return "ERROR";
            case LogLevel::Warning:
                return "WARNING";
            case LogLevel::Info:
                return "INFO";
            case LogLevel::Debug:
                return "DEBUG";
            case LogLevel::Verbose:
                return "VERBOSE";
        }
    }
};

#endif  // logger_h