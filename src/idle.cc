
// Copyright 2014 Toggl Desktop developers

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

#include "../src/idle.h"

#include "./formatter.h"

namespace toggl {

Idle::Idle(GUI *ui)
    : last_idle_seconds_reading_(0)
, last_idle_started_(0)
, last_sleep_started_(0)
, ui_(ui) {
}

void Idle::SetIdleSeconds(
    const Poco::UInt64 idle_seconds,
    User *current_user) {
    /*
    {
        std::stringstream ss;
        ss << "SetIdleSeconds idle_seconds=" << idle_seconds;
        logger().debug(ss.str());
    }
    */

    if (current_user) {
        computeIdleState(idle_seconds, current_user);
    }

    last_idle_seconds_reading_ = idle_seconds;
}

void Idle::computeIdleState(
    const Poco::UInt64 idle_seconds,
    User *current_user) {
    if (settings_.idle_minutes &&
            (idle_seconds >= (settings_.idle_minutes*60)) &&
            !last_idle_started_) {
        last_idle_started_ = time(0) - idle_seconds;

        std::stringstream ss;
        ss << "User is idle since " << last_idle_started_;
        logger().debug(ss.str());

        return;
    }

    if (last_idle_started_ &&
            idle_seconds < last_idle_seconds_reading_) {
        time_t now = time(0);

        TimeEntry *te = current_user->RunningTimeEntry();
        if (!te) {
            logger().warning("Time entry is not tracking, ignoring idleness");
        } else if (TimeEntry::AbsDuration(te->DurationInSeconds())
                   < last_idle_seconds_reading_) {
            logger().warning("Time entry duration is less than idle, ignoring");
        } else if (settings_.use_idle_detection) {
            std::stringstream since;
            since << "You have been idle since "
                  << Formatter::FormatTimeForTimeEntryEditor(
                      last_idle_started_);

            int minutes = static_cast<int>((now - last_idle_started_) / 60);
            std::stringstream duration;
            duration << "(" << minutes << " minute";
            if (minutes != 1) {
                duration << "s";
            }
            duration << ")";

            poco_check_ptr(ui_);
            ui_->DisplayIdleNotification(te->GUID(),
                                         since.str(),
                                         duration.str(),
                                         last_idle_started_);
        }

        std::stringstream ss;
        ss << "User is not idle since " << now;
        logger().debug(ss.str());

        last_idle_started_ = 0;
    }
}

}  // namespace toggl
