#pragma once

#include <cstdint>

#include "dskimage.h"

namespace ein {

// FD1770 floppy disc controller, ports 18-1F (Einstein.zsm:22-27):
//
//   18  command (write) / status (read)
//   19  track      1A  sector      1B  data
//
// Drive and side select are on port 23 (Einstein.zsm:386): bits 0-3 are a
// one-hot drive code -- 01, 02, 04, 08, the table at Mos12.zsm:247 -- and bit 4
// selects side 2. DDEN is tied to ground (Einstein.zsm:396), so the controller
// is permanently in double density: 250 kbit/s MFM, one byte every 32 us.
// INTRQ and DRQ are not wired to the CPU, so MOS polls the status register;
// nothing here raises an interrupt.
//
// Rotation is modelled. A sector read waits for its sector to come round, which
// is what makes read latency depend on head position the way real hardware
// does, and it is what the index pulse in the Type 1 status is derived from --
// ZHMDSC (Mos12.zsm:4113) will not complete a RESTORE until it has seen one.
class Fd1770 {
public:
    // Status bits. Bits 1, 2 and 5 mean different things after a Type 1 command
    // than after a Type 2/3 one; see the table at Einstein.zsm:446-467.
    enum Status : std::uint8_t {
        kBusy = 0x01,
        kDrq = 0x02,             // Type 2/3
        kIndex = 0x02,           // Type 1
        kLostData = 0x04,        // Type 2/3
        kTrack0 = 0x04,          // Type 1
        kCrcError = 0x08,
        kRecordNotFound = 0x10,
        kRecordType = 0x20,      // Type 2/3: 1 = deleted data mark
        kSpinUp = 0x20,          // Type 1: six revolutions completed
        kWriteProtect = 0x40,
        kMotorOn = 0x80,
    };

    static constexpr int kClockHz = 4'000'000;
    static constexpr int kRpm = 300;
    static constexpr int kRevolution = kClockHz * 60 / kRpm;  // 800 000 T-states
    static constexpr int kByteTime = 128;                     // 32 us at 250 kbit/s
    static constexpr int kIndexWidth = kClockHz / 250;        // 4 ms of index pulse
    static constexpr int kSpinUpRevs = 6;    // datasheet motor-on delay
    static constexpr int kMotorOffRevs = 9;  // idle revolutions before the motor stops
    static constexpr int kSettleTStates = kClockHz / 66;  // ~15 ms head settle
    static constexpr int kSearchRevs = 2;    // revolutions before giving up on a sector

    static constexpr int kDrives = 4;

    void reset();

    void save_state(class StateWriter& w) const;
    void load_state(class StateReader& r);

    void tick(int tstates);

    // reg is the low two bits of the port: 0 status/command, 1 track,
    // 2 sector, 3 data.
    std::uint8_t read(int reg);
    void write(int reg, std::uint8_t value);

    // Port 23.
    void select(std::uint8_t value);

    DiskImage& disk(int drive) { return disks_[drive & 3]; }
    const DiskImage& disk(int drive) const { return disks_[drive & 3]; }

    // Debugger accessors.
    std::uint8_t status() const { return compose_status(); }
    std::uint8_t track_reg() const { return track_; }
    std::uint8_t sector_reg() const { return sector_; }
    std::uint8_t data_reg() const { return data_; }
    std::uint8_t last_command() const { return command_; }
    int selected_drive() const { return drive_; }
    int side() const { return side_; }
    int head_track() const { return head_[drive_ < 0 ? 0 : drive_]; }
    bool busy() const { return busy_; }
    bool motor_on() const { return motor_on_; }
    std::uint64_t sectors_read() const { return sectors_read_; }
    std::uint64_t sectors_written() const { return sectors_written_; }
    std::uint64_t not_found() const { return not_found_; }
    const char* phase_name() const;

private:
    enum class Phase { kIdle, kType1, kSearch, kTransfer, kFinish };

    DiskImage disks_[kDrives];

    std::uint8_t status_extra_ = 0;  // sticky error bits for the current result
    std::uint8_t track_ = 0;
    std::uint8_t sector_ = 0;
    std::uint8_t data_ = 0;
    std::uint8_t command_ = 0;
    std::uint8_t head_[kDrives]{};

    int drive_ = -1;  // -1 = none selected
    int side_ = 0;

    bool busy_ = false;
    bool drq_ = false;
    bool type1_ = true;   // which meaning the status bits currently carry
    bool writing_ = false;
    bool motor_on_ = false;

    Phase phase_ = Phase::kIdle;
    int timer_ = 0;
    int rot_ = 0;             // rotational position, 0..kRevolution-1
    int spin_up_left_ = 0;    // T-states of spin-up still to go
    int motor_idle_ = 0;

    // The sector being transferred is held as coordinates, not a pointer, so
    // that changing the disc in a drive can never leave a transfer pointing at
    // freed memory.
    bool xfer_valid_ = false;
    int xfer_track_ = 0;
    int xfer_side_ = 0;
    int xfer_sector_ = 0;
    int xfer_size_ = DiskImage::kSectorSize;
    int xfer_index_ = 0;
    std::uint8_t* xfer_byte(int index);

    std::uint64_t sectors_read_ = 0;
    std::uint64_t sectors_written_ = 0;
    std::uint64_t not_found_ = 0;

    std::uint8_t compose_status() const;
    void begin_command(std::uint8_t cmd);
    void start_motor();
    void advance();
    void finish(std::uint8_t extra);
    int step_rate_tstates(std::uint8_t cmd) const;
    int rotational_delay(int slot_index, int slots) const;
    bool disk_present() const;
};

}  // namespace ein
