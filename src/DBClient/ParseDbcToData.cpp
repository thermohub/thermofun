// ThermoFun includes
#include "ParseDbcToData.h"
#include "Database.h"
#include "Substance.h"
#include "Reaction.h"

namespace ThermoFun {

auto databaseFromRecordList(const DatabaseClient &dbc, const List_VertexId_VertexType &recordList, DefinesSubstLevelOptions levelOptions ) -> Database
{
    std::string _idSubst, _idReact, substSymb;
    std::string level_;
    Database tdb;

    // The set of all aqueous species in the database
    SubstancesMap substances_map;
    // The set of all gaseous species in the database
    ReactionsMap reactions_map;

    // get substances and the reaction symbol if necessary
//    for(auto iterator = recordList.begin(); iterator != recordList.end(); iterator++)
//    {
//        if (iterator->second == "substance")
//        {
//            _idSubst = iterator->first;
//            std::string jsonrecord = dbc.substData().getJsonRecordVertex(_idSubst);
//            auto domdata = jsonio::unpackJson( jsonrecord, "VertexSubstance" ); // with default values
//            domdata->findValue("properties.symbol", substSymb);
//            // record = dbc.substData().getJsonBsonRecordVertex(_idSubst).second;
//            // bsonio::bson_to_key( record.data, "properties.symbol", substSymb );

//            auto mode = levelOptions.definesSubstLevelMode;

//            switch(mode)
//            {
//            case DefinesSubstLevelMode::followAll        : level_ = "-1";  // follows all connected data
//                break;
//            case DefinesSubstLevelMode::followGiven      : level_ = levelOptions.level; // follows edges defines with level
//                break;
//            case DefinesSubstLevelMode::followDiefferentPerSubstance    : {
//                if (levelOptions.substSymbol_definesLevel.find(substSymb) != levelOptions.substSymbol_definesLevel.end()) // follows edges defines with specific leveles for substSymbols
//                    level_ = levelOptions.substSymbol_definesLevel[substSymb];   // if the substance symbol is not found in the map, it uses the default level
//                else
//                    level_ = levelOptions.level;
//            }
//                break;
//            }

//            Substance substance = parseSubstance(domdata.get());

//            // get reaction symbol which define substance with _idSubst
//            std::string definesReactSymb = dbc.substData().definesReactionSymbol(_idSubst, level_);
//            if (definesReactSymb != "")
//            {
//                substance.setReactionSymbol(definesReactSymb);
//                substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
//            }

//            if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
//                substances_map[substance.symbol()] = substance;
//            } else {
//                errorSameSymbol("substance", substance.symbol(), __LINE__, __FILE__ );
//            }
//        } else
//            if (iterator->second == "reaction")
//            {
//                _idReact = iterator->first;
//                std::string jsonrecord = dbc.reactData().getJsonRecordVertex(_idReact);
//                auto domdata = jsonio::unpackJson( jsonrecord, "VertexReaction" ); // with default values
//                // record = dbc.reactData().getJsonBsonRecordVertex(_idReact).second;

//                Reaction reaction = ThermoFun::parseReaction(domdata.get());

//                // get reactants by following reaction incoming takes edge
//                reaction.setReactants(dbc.reactData().reactantsCoeff(_idReact));

//                if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
//                    reactions_map[reaction.symbol()] = reaction;
//                } else {
//                    errorSameSymbol("reaction", reaction.symbol(), __LINE__, __FILE__ );
//                }
//            }
//    }
    tdb.addMapReactions(reactions_map);
    tdb.addMapSubstances(substances_map);
    return tdb;
}

}
