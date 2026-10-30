// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_ELEMENTRECORD_H
#define THERMOFUN_MODEL_ELEMENTRECORD_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Model/DataFormat.h>
#include <ThermoFun/Model/Quantity.h>
#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::v2 {

/// An element of the database, as the file says.
struct ElementRecord
{
    std::string symbol;                 ///< "Fe", or "Zz" for the charge
    std::string name;
    ElementClass class_ = ElementClass::Element;
    std::optional<int> isotope_mass;
    std::optional<int> number;          ///< atomic number
    std::optional<int> valence;
    std::optional<Quantity> molar_mass;          ///< kg/mol (the legacy key atomic_mass is in g/mol)
    std::optional<Quantity> standard_entropy;    ///< J/(mol.K) (the legacy key entropy)
    std::optional<Quantity> heat_capacity;       ///< J/(mol.K)
    std::optional<Quantity> volume;              ///< m3/mol
    std::string reference_form;                  ///< for example "graphite"
    std::map<std::string, Quantity> structural;  ///< more data, for example ionic_radius
    std::vector<std::string> data_sources;
    std::string sourcetdb;
    std::string extra_json;                      ///< fields that the format does not define, as JSON text
    DataFormat source_format = DataFormat::Legacy;
    std::string original_text;                   ///< the text of the record as it was read

    auto operator==(const ElementRecord&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
