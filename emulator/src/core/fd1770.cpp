#include "fd1770.h"

#include "state.h"

namespace ein {

void Fd1770::reset() {
    status_extra_ = 0;
    track_ = sector_ = data_ = command_ = 0;
    for (auto& h : head_) h = 0;
    drive_ = -1;
    side_ = 0;
    busy_ = drq_ = writing_ = motor_on_ = false;
    type1_ = true;
    phase_ = Phase::kIdle;
    timer_ = 0;
    rot_ = 0;
    spin_up_left_ = 0;
    motor_idle_ = 0;
    xfer_valid_ = false;
    xfer_index_ = 0;
    sectors_read_ = sectors_written_ = not_found_ = 0;
}

// The controller, not the media: a state saved with a disc in drive 0 is
// restored into whatever disc is in drive 0 at the time.
void Fd1770::save_state(StateWriter& w) const {
    w.put(status_extra_);
    w.put(track_);
    w.put(sector_);
    w.put(data_);
    w.put(command_);
    w.bytes(head_, sizeof head_);
    w.put(drive_);
    w.put(side_);
    w.put(busy_);
    w.put(drq_);
    w.put(type1_);
    w.put(writing_);
    w.put(motor_on_);
    w.put(phase_);
    w.put(timer_);
    w.put(rot_);
    w.put(spin_up_left_);
    w.put(motor_idle_);
    w.put(xfer_valid_);
    w.put(xfer_track_);
    w.put(xfer_side_);
    w.put(xfer_sector_);
    w.put(xfer_size_);
    w.put(xfer_index_);
}

void Fd1770::load_state(StateReader& r) {
    r.get(status_extra_);
    r.get(track_);
    r.get(sector_);
    r.get(data_);
    r.get(command_);
    r.bytes(head_, sizeof head_);
    r.get(drive_);
    r.get(side_);
    r.get(busy_);
    r.get(drq_);
    r.get(type1_);
    r.get(writing_);
    r.get(motor_on_);
    r.get(phase_);
    r.get(timer_);
    r.get(rot_);
    r.get(spin_up_left_);
    r.get(motor_idle_);
    r.get(xfer_valid_);
    r.get(xfer_track_);
    r.get(xfer_side_);
    r.get(xfer_sector_);
    r.get(xfer_size_);
    r.get(xfer_index_);
}

bool Fd1770::disk_present() const {
    return drive_ >= 0 && disks_[drive_].loaded();
}

const char* Fd1770::phase_name() const {
    switch (phase_) {
        case Phase::kIdle: return "idle";
        case Phase::kType1: return "seek";
        case Phase::kSearch: return "search";
        case Phase::kTransfer: return writing_ ? "write" : "read";
        case Phase::kFinish: return "finish";
    }
    return "?";
}

// The status register is a different register depending on what the last
// command was, which is why type1_ is tracked. Einstein.zsm:446-467.
std::uint8_t Fd1770::compose_status() const {
    std::uint8_t s = status_extra_;
    if (motor_on_) s |= kMotorOn;
    if (busy_) s |= kBusy;
    if (type1_) {
        if (spin_up_left_ == 0 && motor_on_) s |= kSpinUp;
        if (drive_ >= 0 && head_[drive_] == 0) s |= kTrack0;
        // The index hole passes once per revolution.
        if (disk_present() && rot_ < kIndexWidth) s |= kIndex;
        if (disk_present() && disks_[drive_].write_protected) s |= kWriteProtect;
    } else {
        if (drq_) s |= kDrq;
        if (writing_ && disk_present() && disks_[drive_].write_protected) s |= kWriteProtect;
    }
    return s;
}

std::uint8_t Fd1770::read(int reg) {
    switch (reg & 3) {
        case 0: return compose_status();
        case 1: return track_;
        case 2: return sector_;
        default:
            // Reading the data register acknowledges the byte.
            drq_ = false;
            return data_;
    }
}

void Fd1770::write(int reg, std::uint8_t value) {
    switch (reg & 3) {
        case 0: begin_command(value); return;
        case 1: track_ = value; return;
        case 2: sector_ = value; return;
        default:
            data_ = value;
            if (busy_ && writing_ && drq_) {
                if (std::uint8_t* b = xfer_byte(xfer_index_)) *b = value;
                ++xfer_index_;
                drq_ = false;
            }
            return;
    }
}

void Fd1770::select(std::uint8_t value) {
    // One-hot drive code in bits 0-3; anything else deselects.
    drive_ = -1;
    for (int i = 0; i < kDrives; ++i) {
        if (value & (1u << i)) { drive_ = i; break; }
    }
    side_ = (value & 0x10) ? 1 : 0;
}

void Fd1770::start_motor() {
    if (!motor_on_) {
        motor_on_ = true;
        // Six revolutions before the chip considers the disc up to speed. A
        // command issued with h=0 waits this out before it executes.
        spin_up_left_ = kSpinUpRevs * kRevolution;
    }
    motor_idle_ = 0;
}

int Fd1770::step_rate_tstates(std::uint8_t cmd) const {
    // r1 r0: 6, 12, 20, 30 ms (Einstein.zsm:420).
    static const int kMs[4] = {6, 12, 20, 30};
    return kMs[cmd & 3] * (kClockHz / 1000);
}

// T-states until the given physical slot passes under the head. The slot comes
// from the track's own sector order, so an interleaved disc -- Oh Mummy is laid
// out 0,5,1,6,2,7,3,8,4,9, a 2:1 interleave -- reads at the speed that
// interleave was chosen to give.
int Fd1770::rotational_delay(int slot_index, int slots) const {
    if (slots <= 0) slots = DiskImage::kSectorsPerTrack;
    if (slot_index < 0) slot_index = 0;
    const int slot = kRevolution / slots;
    const int target = slot * slot_index;
    int delay = target - rot_;
    if (delay < 0) delay += kRevolution;
    return delay;
}

void Fd1770::begin_command(std::uint8_t cmd) {
    command_ = cmd;

    // Type 4, force interrupt: terminates whatever is running. I3-I0 = 0 means
    // "terminate without interrupt", which is how MOS aborts after an error
    // (Mos12.zsm:4067).
    if ((cmd & 0xf0) == 0xd0) {
        busy_ = false;
        drq_ = false;
        phase_ = Phase::kIdle;
        xfer_valid_ = false;
        type1_ = true;
        return;
    }

    start_motor();
    status_extra_ = 0;
    drq_ = false;
    busy_ = true;
    xfer_valid_ = false;
    xfer_index_ = 0;

    // Commands issued with the h flag clear wait for spin-up first.
    const int spin_wait = (cmd & 0x08) ? 0 : spin_up_left_;

    if ((cmd & 0x80) == 0) {
        // ---- Type 1: RESTORE, SEEK, STEP, STEP-IN, STEP-OUT.
        type1_ = true;
        writing_ = false;
        const int drive = drive_ < 0 ? 0 : drive_;
        int steps = 0;
        if ((cmd & 0xf0) == 0x00) {
            // RESTORE steps out until the drive asserts TR00, so the count
            // comes from where the head physically is, not from the register.
            steps = head_[drive];
            head_[drive] = 0;
            track_ = 0;
        } else {
            // Everything else works off the track register, which is what the
            // chip itself compares against.
            int target = track_;
            switch (cmd & 0xf0) {
                case 0x10: target = data_; break;       // SEEK
                case 0x20: case 0x30: break;            // STEP, same direction
                case 0x40: case 0x50: ++target; break;  // STEP-IN
                default: --target; break;               // STEP-OUT
            }
            if (target < 0) target = 0;
            if (target > 255) target = 255;
            steps = (target > track_) ? target - track_ : track_ - target;
            const int moved = target - track_;
            int head = head_[drive] + moved;
            if (head < 0) head = 0;
            if (head > 255) head = 255;
            head_[drive] = static_cast<std::uint8_t>(head);
            // SEEK always updates the track register; STEP only when u is set.
            if ((cmd & 0xf0) == 0x10 || (cmd & 0x10)) {
                track_ = static_cast<std::uint8_t>(target);
            }
        }

        timer_ = spin_wait + steps * step_rate_tstates(cmd);
        if (cmd & 0x04) timer_ += kSettleTStates;  // V: verify the destination
        phase_ = Phase::kType1;
        if (timer_ <= 0) timer_ = 1;
        return;
    }

    // ---- Type 2/3.
    type1_ = false;
    const std::uint8_t kind = cmd & 0xf0;
    writing_ = (kind == 0xa0) || (kind == 0xf0);

    if (writing_ && disk_present() && disks_[drive_].write_protected) {
        finish(kWriteProtect);
        return;
    }

    if (kind == 0x80 || kind == 0x90 || kind == 0xa0 || kind == 0xb0) {
        // READ SECTOR / WRITE SECTOR.
        const int drive = drive_ < 0 ? 0 : drive_;
        const DiskImage::Sector* sec =
            disk_present() ? disks_[drive].find(head_[drive], side_, sector_) : nullptr;
        if (!sec) {
            // Nothing matching: the chip searches for a few revolutions before
            // reporting Record Not Found.
            timer_ = spin_wait + kSearchRevs * kRevolution;
            phase_ = Phase::kSearch;
            xfer_valid_ = false;
            return;
        }
        xfer_valid_ = true;
        xfer_track_ = head_[drive];
        xfer_side_ = side_;
        xfer_sector_ = sector_;
        xfer_size_ = sec->size;
        timer_ = spin_wait +
                 rotational_delay(disks_[drive].slot_of(head_[drive], side_, sector_),
                                  static_cast<int>(disks_[drive].track(head_[drive], side_).size()));
        if (cmd & 0x04) timer_ += kSettleTStates;  // E: 30 ms settling
        phase_ = Phase::kSearch;
        if (timer_ <= 0) timer_ = 1;
        return;
    }

    // READ ADDRESS, READ TRACK and WRITE TRACK are not modelled: MOS never
    // issues them, and formatting is an M9 concern. Report Record Not Found
    // rather than hanging the controller.
    timer_ = spin_wait + kRevolution;
    phase_ = Phase::kSearch;
    xfer_valid_ = false;
}

// Resolves the byte currently being transferred. Returns nullptr if the disc
// went away underneath the transfer, which aborts it as a lost-data error.
std::uint8_t* Fd1770::xfer_byte(int index) {
    if (!xfer_valid_ || drive_ < 0 || index < 0 || index >= xfer_size_) return nullptr;
    std::uint8_t* s = disks_[drive_].sector(xfer_track_, xfer_side_, xfer_sector_);
    return s ? s + index : nullptr;
}

void Fd1770::finish(std::uint8_t extra) {
    status_extra_ |= extra;
    busy_ = false;
    drq_ = false;
    phase_ = Phase::kIdle;
    xfer_valid_ = false;
    timer_ = 0;
}

void Fd1770::advance() {
    switch (phase_) {
        case Phase::kType1: {
            // Verify (V) needs a readable ID field on the destination track.
            std::uint8_t extra = 0;
            if (command_ & 0x04) {
                const int drive = drive_ < 0 ? 0 : drive_;
                const bool ok =
                    disk_present() && !disks_[drive].track(head_[drive], side_).empty();
                if (!ok) extra |= kRecordNotFound;
            }
            finish(extra);
            return;
        }

        case Phase::kSearch:
            if (!xfer_valid_) {
                ++not_found_;
                finish(kRecordNotFound);
                return;
            }
            phase_ = Phase::kTransfer;
            xfer_index_ = 0;
            timer_ = kByteTime;
            if (writing_) drq_ = true;  // the chip asks for the first byte
            return;

        case Phase::kTransfer:
            if (writing_) {
                if (xfer_index_ >= xfer_size_) {
                    ++sectors_written_;
                    phase_ = Phase::kFinish;
                    timer_ = 2 * kByteTime;  // CRC
                    return;
                }
                // The CPU missed the window for this byte.
                if (drq_) status_extra_ |= kLostData;
                drq_ = true;
                timer_ = kByteTime;
                return;
            }
            if (xfer_index_ >= xfer_size_) {
                ++sectors_read_;
                phase_ = Phase::kFinish;
                timer_ = 2 * kByteTime;  // CRC
                return;
            }
            // A byte the CPU never collected is lost data.
            if (drq_) status_extra_ |= kLostData;
            if (const std::uint8_t* b = xfer_byte(xfer_index_)) {
                data_ = *b;
            } else {
                finish(kLostData);
                return;
            }
            ++xfer_index_;
            drq_ = true;
            timer_ = kByteTime;
            return;

        case Phase::kFinish:
            finish(0);
            return;

        case Phase::kIdle:
        default:
            busy_ = false;
            timer_ = 0;
            return;
    }
}

void Fd1770::tick(int tstates) {
    rot_ += tstates;
    while (rot_ >= kRevolution) rot_ -= kRevolution;

    if (motor_on_) {
        if (spin_up_left_ > 0) {
            spin_up_left_ -= tstates;
            if (spin_up_left_ < 0) spin_up_left_ = 0;
        }
        if (busy_) {
            motor_idle_ = 0;
        } else {
            motor_idle_ += tstates;
            if (motor_idle_ >= kMotorOffRevs * kRevolution) {
                motor_on_ = false;
                spin_up_left_ = 0;
            }
        }
    }

    if (!busy_) return;
    timer_ -= tstates;
    while (busy_ && timer_ <= 0) {
        // How far past the deadline we already are; carried into the next
        // interval so byte pacing does not drift when tick() is called with
        // more T-states than one byte takes.
        const int overshoot = timer_;
        advance();
        if (!busy_) break;
        timer_ += overshoot;
    }
}

}  // namespace ein
