#pragma once
#include <algorithm>

namespace fakelag {
inline constexpr int MaxNewCommands = 15;

// Hold a fakelag goal for the complete command batch. Preparing a command
// does not finish its batch; only the final packet decision advances it.
class ChokeCycle {
public:
    void Reset() { *this = {}; }
    template<class Random>
    int Prepare(int command, int type, int minimum, int maximum, int requested, Random random) {
        if (command <= 0) { Reset(); return 0; }
        type = type >= 0 && type <= 3 ? type : 0;
        maximum = std::clamp(maximum,0,MaxNewCommands-1);
        minimum = std::clamp(minimum,0,maximum);
        if (command < prepared || type != mode || minimum != low || maximum != high) {
            Reset(); mode=type; low=minimum; high=maximum;
        }
        if (command == prepared) return target;
        prepared=command;
        if (needsTarget) {
            target = mode == 2 ? (nextHigh ? high : low) : mode == 3 ? random(low,high) : requested;
            target=std::clamp(target,low,high); needsTarget=false;
        }
        return target;
    }
    void Commit(int command, bool sent) {
        if (!sent || command != prepared || command <= committed) return;
        committed=command; needsTarget=true;
        if (mode == 2) nextHigh=!nextHigh;
    }
private:
    int prepared{}, committed{}, target{}, mode=-1, low=-1, high=-1;
    bool needsTarget=true, nextHigh=true;
};

}
