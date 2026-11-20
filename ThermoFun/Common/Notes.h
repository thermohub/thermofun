// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_NOTES_H
#define THERMOFUN_COMMON_NOTES_H

// Notes on a result (decision 0029): why a value can be doubted, and where it came from. They replace the free text of the
// status message, which is now made from them (render). A note of a method that another one uses travels with the result:
// a solute gets the notes of the water it was calculated on, with the path in `via`.

#include <string>
#include <vector>

namespace ThermoFun {

struct Note
{
    enum class Kind
    {
        Range,  ///< a value outside the range of a model, or of the data of a record
    };
    enum class Severity
    {
        Soft,  ///< the value is calculated: it can be used, with care
        Hard,  ///< the model gives no value
    };

    Kind kind = Kind::Range;
    Severity severity = Severity::Soft;
    std::string method;            ///< the method that made the note, for example "JohnsonNorton91"
    std::string text;              ///< what is outside, and of what, for example "T = 1000.85 degC, P = 5000 bar, above 1000 degC or 5000 bar"
    std::vector<std::string> via;  ///< the path of an inherited note, nearest first, for example {"the water"}

    auto operator==(const Note&) const -> bool = default;
};

using Notes = std::vector<Note>;

/// The words that start the text of a range note in a status message (Common/OutsideRange.h finds them).
inline constexpr const char* rangeNoteMark = "Outside the range of the model: ";

/// The text of one note in a status message, as the methods wrote it before the notes: "Outside the range of the model:
/// via the water: JohnsonNorton91: T = ...; "
inline auto render(const Note& note) -> std::string
{
    std::string out = rangeNoteMark;
    for (const auto& step : note.via)
        out += "via " + step + ": ";
    if (!note.method.empty())
        out += note.method + ": ";
    return out + note.text + "; ";
}

/// The status message made from the notes.
inline auto render(const Notes& notes) -> std::string
{
    std::string out;
    for (const auto& note : notes)
        out += render(note);
    return out;
}

/// Add a note, once.
inline auto addNote(Notes& notes, const Note& note) -> void
{
    for (const auto& n : notes)
        if (n == note)
            return;
    notes.push_back(note);
}

/// Add the notes of a result that this one is calculated from, with the step of the path, for example "the water".
inline auto inheritNotes(Notes& notes, const Notes& from, const std::string& step) -> void
{
    for (auto note : from)
    {
        note.via.insert(note.via.begin(), step);
        addNote(notes, note);
    }
}

} // namespace ThermoFun

#endif
