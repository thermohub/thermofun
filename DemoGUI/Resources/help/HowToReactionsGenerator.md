# Reactions Generator
* [How to use the Reactions Generator](#How.to.generate.reactions.using.the.Reactions.Generator)
* [Application Menu](#Application.Menu)

Given a list of substances and selected master substances, the reactions generator will generate a set of independent reactions. One reaction for each reaction dependent substance. 

![Reactions Generator window](images/ReacGenEmpty.png =840x "Reactions Generator window")
_Empty Reactions Generator Window_

## How to generate reactions using the Reactions Generator 
> __Note__  
Follow the messages written in the status bar for the work flow, success, and error messages. 

__1.__ To start using the Reactions Generator Application first click the [Select Database source and Elements](#Source.Database.Elements) ![Source Database Elements](images/SourceElementFilter.png "Source Database Elements"). Chose the desired source database (_Step 1_), in the next step the desired elements (_Step 2_), and in the last step if present the desired reaction sets (_Step 3_). This will filter the data available for generating reactions.  

> __Note__  
The available elements are based on the available substances in the selected source database. If a substance contains an element, the element will be selectable.  

| ![Select Source Database](images/SelectSource.png =x250 "Select Source Database")  | ![Select Elements](images/SelectElements.png =x250 "Select Elements")  | ![Select Reaction Sets](images/SelectReactionSets.png =x250 "Select Reaction Sets") |
|::|::|::|
| _Step 1_ | _Step 2_  | _Step 3_  |

__2.__ Select the substances to use for generating reactions (default - all substances are selected) by clicking the [Select Substances](#Select.Substances) ![Select Substances](images/SelectData.png "Select Substances"). Clear all to deselect all. Highlighted means selected. After selecting, the SUBSTANCES list on the left should contain only the selected substances.

![Select Substances](images/SelectSubstances.png =420x "Select Substances")  
_Select Substances_

__3.__ Select the master substances by clicking the [Select Master Substances](#Select.Master.Substances) ![Select Master Substances](images/SelectMaster.png "Select Master Substances"). After clicking OK, the "Type" in the SUBSTANCES list on the left side, should be set to "__M__" for the master substances, and to "__D__" for the reaction dependent substances.

> __Note__  
The number of master substances is given by the rank (__R__) of the elements stoichiometry matrix which results from the selected substances. The reactions generator algorithm will always try to have as master species the first R substances in the list. 

> If the user selects more master substances than possible, the algorithm will eliminate master species from the bottom of the selected list. 

> If a selected master substance can not be set as master it will be automatically switched with a different substance during the process of generating reactions. 

![Select Master Substances](images/SelectMasterSubst.png =420x "Select Master Substances")  
_Select Master Substances_

__4.__ Generate reactions by clicking [Generate Reactions](#Generate.Reactions) ![Generate Reactions](images/ReacDCModuleIcon24.png "Generate Reactions"). If the reactions are generated succesfuly they are displayed in the window, together with their calculated properties for 1 bar and 25 C.

![Reactions Generated](images/ReactGenerated.png =840x "Reactions Generated")  
_Reactions were generated successfully_ 

__5.__ Save the generated reactions in a Reactions Set (with __Symbol__, __Name__, __Comment__, __Set Type__, and __Reaction Level__) by clicking [Save Reactions Set](#Save.Reactions.Set) ![Save Reactions Set](images/SaveCurrentRecordIcon.png "Save Reactions Set"). Before saving set the desired Reactions Set __Symbol__, __Name__, __Comment__, __Set Type__, and __Reaction Level__. If the reactions are saved successfully they should be visible when browsing the reaction data inside the database. 

> __Note__  
When saving the generated reactions, the symbol of the saved reactions is set the same as the symbol of the reaction defined substance. For example: Ca+2 + CO3-2 = CaCO3@ will have the symbol CaCO3@. 

> When saving the reactions, if a reaction with the same symbol is already exists in the database the symbol of the saved reaction will be the symbol of the defined substance underscore _ and the level number. For example: CaCO3@_1. 

![Save Reactions](images/SaveReactions.png =840x "Save Reactions")  
_All set for saving the generated reactions_

![Saved Reactions Set](images/SavedGenReacSet.png =840x "Saved Reactions Set")  
_A data entry in the database for a saved Reactions set_

![Saved Reactions in the database](images/SavedGenReact.png =840x "Saved Reactions in the database")  
_Entries in the database for saved generated reactions_

## Application Menu 
* __File__
  * ![New.Reset](images/NewCreateRecordIcon24.png "New/Reset") __New/Reset__: Resets the Reactions Generator window. 
  * ![Load.Reactions](images/ReactionDatabaseIcon24.png "Load Reactions") __Load Reactions__: Load reactions from the database which will be decomposed in a list of substances which participate in these reactions. The resulted list of substances can be further used to generate reactions.  
  * ![Save.Reactions.Set](images/SaveCurrentRecordIcon.png "Save Reactions Set") __Save Reaction Set__: Save generated reactions in the database as a Reactions Set. Set the Reaction Set __Symbol__, __Name__, __Comment__, __Set Type__, and __Reaction Level__ in the respective fields. 
* __Data__
  * ![Source.Database.Elements](images/SourceElementFilter.png "Source Database Elements") __Select Data Source and Elements__: Opens a wizard for selecting the source database and elements for filtering the data to be used for generating reactions. Select the source database from the available list of source databases. Next, select the elements which the substances and reactions should contain. 
  * ![Select.Substances](images/SelectData.png "Select Substances") __Select Substances__: Select the list of substances to be used for generating reactions. 
  * ![Select.Master.Substances](images/SelectMaster.png "Select Master Substances") __Select Master Substances__: Select the list of desired master substances which will be used  for generating reaction. 
* __Tools__
  * ![Generate.Reactions](images/ReacDCModuleIcon24.png "Generate Reactions") __Generate Reactions__: Generate reactions form the selected substances.