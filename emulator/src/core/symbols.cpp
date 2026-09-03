#include "symbols.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>

namespace ein {
namespace {

std::string trim(const std::string& s) {
    std::size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) --b;
    return s.substr(a, b - a);
}

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Splits a source line into code and comment, respecting quoted strings so a
// ';' inside a message is not mistaken for the start of a comment.
void split_comment(const std::string& in, std::string& code, std::string& comment) {
    bool quoted = false;
    for (std::size_t i = 0; i < in.size(); ++i) {
        const char c = in[i];
        if (c == '\'') quoted = !quoted;
        if (c == ';' && !quoted) {
            code = in.substr(0, i);
            comment = trim(in.substr(i + 1));
            return;
        }
    }
    code = in;
    comment.clear();
}

// Splits on commas at the top level, keeping quoted strings whole.
std::vector<std::string> split_items(const std::string& s) {
    std::vector<std::string> out;
    bool quoted = false;
    std::string cur;
    for (char c : s) {
        if (c == '\'') quoted = !quoted;
        if (c == ',' && !quoted) {
            out.push_back(trim(cur));
            cur.clear();
            continue;
        }
        cur.push_back(c);
    }
    if (!trim(cur).empty()) out.push_back(trim(cur));
    return out;
}

bool is_hex_label(const std::string& name, int& addr) {
    if (name.size() != 5 || (name[0] != 'L' && name[0] != 'l')) return false;
    for (int i = 1; i < 5; ++i) {
        if (!std::isxdigit(static_cast<unsigned char>(name[i]))) return false;
    }
    addr = static_cast<int>(std::strtol(name.substr(1).c_str(), nullptr, 16));
    return true;
}

// Byte length of one DEFB/DEFW item.
int item_bytes(const std::string& item, int per) {
    if (!item.empty() && item[0] == '\'') {
        // A quoted string contributes one byte per character.
        int n = 0;
        for (std::size_t i = 1; i < item.size() && item[i] != '\''; ++i) ++n;
        return n;
    }
    return per;
}

}  // namespace

bool Symbols::parse_number(const std::string& tok_in, int& out) const {
    std::string tok = trim(tok_in);
    if (tok.empty()) return false;
    const std::string up = upper(tok);
    if (up.back() == 'H' && up.size() > 1) {
        char* end = nullptr;
        const long v = std::strtol(up.substr(0, up.size() - 1).c_str(), &end, 16);
        if (end && *end == '\0') {
            out = static_cast<int>(v);
            return true;
        }
        return false;
    }
    if (std::isdigit(static_cast<unsigned char>(up[0]))) {
        char* end = nullptr;
        const long v = std::strtol(up.c_str(), &end, 10);
        if (end && *end == '\0') {
            out = static_cast<int>(v);
            return true;
        }
        return false;
    }
    const auto it = equates_.find(up);
    if (it != equates_.end()) {
        out = it->second;
        return true;
    }
    return false;
}

void Symbols::load_equates(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f) return;
    std::string raw;
    while (std::getline(f, raw)) {
        std::string code, comment;
        split_comment(raw, code, comment);
        code = trim(code);
        if (code.empty()) continue;

        // NAME EQU value, with the name optionally followed by a colon.
        const std::string up = upper(code);
        const std::size_t eq = up.find("EQU");
        if (eq == std::string::npos || eq == 0) continue;
        std::string name = trim(code.substr(0, eq));
        if (!name.empty() && name.back() == ':') name.pop_back();
        name = trim(name);
        if (name.empty()) continue;
        const std::string value = trim(code.substr(eq + 3));
        int v = 0;
        if (!parse_number(value, v)) continue;
        equates_[upper(name)] = v;
    }
}

void Symbols::load_source(const std::filesystem::path& path, const ReadByte& read) {
    std::ifstream f(path);
    if (!f) return;
    const std::string filename = path.filename().string();

    int pc = -1;   // -1 until the first anchor
    int line_no = 0;
    std::string raw;
    while (std::getline(f, raw)) {
        ++line_no;
        std::string code, comment;
        split_comment(raw, code, comment);
        if (trim(code).empty()) continue;

        // A label starts in column 0.
        std::string label;
        std::string body = code;
        if (!code.empty() && code[0] != ' ' && code[0] != '\t') {
            const std::size_t colon = code.find(':');
            std::size_t ws = code.find_first_of(" \t");
            if (colon != std::string::npos && (ws == std::string::npos || colon < ws)) {
                label = trim(code.substr(0, colon));
                body = code.substr(colon + 1);
            } else if (ws != std::string::npos) {
                label = trim(code.substr(0, ws));
                body = code.substr(ws);
            } else {
                label = trim(code);
                body = "";
            }
        }
        body = trim(body);

        // Mnemonic and operands.
        std::string mnemonic = body;
        std::string operands;
        const std::size_t ws = body.find_first_of(" \t");
        if (ws != std::string::npos) {
            mnemonic = trim(body.substr(0, ws));
            operands = trim(body.substr(ws));
        }
        const std::string mn = upper(mnemonic);

        // EQU lines define a symbol and occupy no space.
        if (mn == "EQU") {
            int v = 0;
            if (!label.empty() && parse_number(operands, v)) equates_[upper(label)] = v;
            continue;
        }

        // ORG is an absolute anchor.
        if (mn == "ORG") {
            int v = 0;
            if (parse_number(operands, v)) pc = v & 0xffff;
            continue;
        }

        // An Lxxxx label carries its own address, and is the anchor that keeps
        // the walk honest.
        if (!label.empty()) {
            int at = 0;
            if (is_hex_label(label, at)) {
                ++anchors_;
                if (pc == at) ++anchors_matched_;
                pc = at;
            }
            if (pc >= 0) {
                // Named labels take whatever address the walk has reached; the
                // Lxxxx ones already know their own.
                labels_[static_cast<std::uint16_t>(pc)] = label;
                label_addr_[upper(label)] = pc;
            }
        }

        if (mn.empty() || pc < 0) continue;
        if (mn == "END" || mn == "TITLE" || mn == "SUBTTL" || mn[0] == '*' || mn == "IF" ||
            mn == "ENDIF" || mn == "MACRO" || mn == "ENDM") {
            continue;
        }

        // Length: from the source for data, from the real bytes for code.
        int length = 0;
        bool is_data = true;
        if (mn == "DEFB" || mn == "DB" || mn == "DEFM") {
            for (const std::string& it : split_items(operands)) length += item_bytes(it, 1);
        } else if (mn == "DEFW" || mn == "DW") {
            for (const std::string& it : split_items(operands)) length += item_bytes(it, 2);
        } else if (mn == "DC") {
            // A string whose last byte has bit 7 set; the length is the string.
            for (const std::string& it : split_items(operands)) length += item_bytes(it, 1);
        } else if (mn == "DEFS" || mn == "DS") {
            int v = 0;
            if (parse_number(operands, v)) length = v;
        } else {
            is_data = false;
            length = instruction_length(read, static_cast<std::uint16_t>(pc));
        }
        if (length <= 0) length = 1;

        Line l;
        l.addr = static_cast<std::uint16_t>(pc);
        l.label = label;
        l.text = body;
        l.comment = comment;
        l.file = filename;
        l.line_no = line_no;
        l.is_data = is_data;
        lines_[l.addr] = std::move(l);

        pc = (pc + length) & 0xffff;
    }
}

const Symbols::Line* Symbols::line_at(std::uint16_t addr) const {
    const auto it = lines_.find(addr);
    return it == lines_.end() ? nullptr : &it->second;
}

const std::string* Symbols::label_at(std::uint16_t addr) const {
    const auto it = labels_.find(addr);
    return it == labels_.end() ? nullptr : &it->second;
}

int Symbols::lookup(const std::string& name) const {
    // A code label wins over an equate of the same name.
    const std::string up = upper(name);
    const auto l = label_addr_.find(up);
    if (l != label_addr_.end()) return l->second;
    const auto it = equates_.find(up);
    return it == equates_.end() ? -1 : it->second;
}

}  // namespace ein
