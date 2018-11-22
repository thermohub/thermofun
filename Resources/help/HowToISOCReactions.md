# Isocoulombic Reactions Generator
* [How to use the isocoulombic reactions generator](#How.to.generate.isocoulombic.reactions)
* [Application Menu](#Application.Menu)

Given a list of selected non isocoulombic reactions and a list of selected model reactions, the isocoulombic reactions generator will generate all possible isocoulombic reactions, by combining the non isocoulombic reactions with the model reactions. The logK values for the non isocoulombic reactions can be then calculated by using the isocoulombic reactions using one-, two- or three-term extrapolation methods. 

![Isocoulombic Reactions Generator Window](images/IsocGenWidgetEmpty.png =840x "Isocoulombic Reactions Generator Window")  
_Empty Isocoulombic Reactions Generator Window_

## How to generate isocoulombic reactions
> __Note__  
Follow the messages written in the status bar for the work flow, success, and error messages. 

__1.__ To start using the Isocoulombic Reactions Generator Application first click the [Select Database source and Elements](#Source.Database.Elements) ![Source Database Elements](images/SourceElementFilter.png "Source Database Elements"). Chose the desired source database (_Step 1_), in the next step the desired elements (_Step 2_), and in the last step if present the desired reaction sets (_Step 3_). This will filter the data available for generating reactions.  

> __Note__  
The available elements are based on the available substances in the selected source database. If a substance contains an element, the element will be selectable.  

| ![Select Source Database](images/SelectSource.png =x250 "Select Source Database")  | ![Select Elements](images/SelectElements.png =x250 "Select Elements")  | ![Select Reaction Sets](images/SelectReactionSets.png =x250 "Select Reaction Sets") |
|::|::|::|
| _Step 1_ | _Step 2_  | _Step 3_  |

__2.__ Select one or more non-isocoulombic reactions to be investigated, by clicking [Select Investigated Reactions](#) ![Select.Investigate.Reactions](images/SelectData.png "Select Investigated Reactions").

![Select Investigated Reactions](images/SelectInvReact.png =420x"Select Investigated Reactions")  
_Select Investigated reaction/s_

__3.__ Select the model reactions which will be used for generating isocoulombic reactions, by clicking [Select Model Reaction](#) ![Select Model Reactions](images/SelectMaster.png "Select Model Reactions"). 

![Select Model Reactions](images/SelectModelReact.png =420x"Select Model Reactions")  
_Select Model Reactions_

__4.__ Generate isocoulombic reactions, by clicking [Generate Isocoulombic Reaction](#) ![Generate Isocoulombic Reactions](images/ReacDCModuleIcon24.png "Generate Isocoulombic Reactions"). If the reactions were successfully generated, they will be displayed in the window, together with their properties at 25 C 1 bar. 

You can now calculate the logK values for the investigated reactions (or the isocoulombic reactions) using the one-, two-, and three-term extrapolation methods. Or you can now save a selection from the generated isocoulombic reactions as reaction sets. 

![Successfully generated isocoulombic reactions](images/IsocGen.png =840x"Successfully generated isocoulombic reactions")  
_Successfully generated isocoulombic reactions_

__5.__ Calculated logK values for the investigated reactions at different temperature and pressure points, by clicking [Calculate Properties of Invesitgated Reactions](#) ![Calculate Properties of Invesitgated Reactions](images/CalculateRecrod.png "Calculate Properties of Invesitgated Reactions"). _Step 1_ , define the temperature and pressure points at which the logK values will be calculated. _Step 2_,  select the isocoulombic reactions which will be used in the calculations. _Step 3_, select one or more extrapolation method to be used when doing the calculations using the previously selected isocoulombic reactions.

> __Note__  
The selected extrapolation method/s if possible (depending on the thermodynamic data available) will be automatically set for all the selected isocoulombic reactions. 

> Based on the selected isocoulombic reactions and the selected extrapolation methods in the results file a collumn with the name: \extrapolation type\_\number of the investigated reaction\_\reaction equation. For example: __1T_0_CeCl+2+Cm+3+NO3-=Ce+3+Cl-+Cam(NO3)+2__, this means that the values in found in this column were calculated using the one-term extrapolation method and the isocoulombic reaction with the given equation (index 0, first investigated reaction in the investigated reactions list). 

> 1T - one-term; 2T - two-term; 3T -three-term;

| ![Select temperature and pressure points](images/SelectTP.png =x250 "Select temperature and pressure points") | ![Select Isocoulombic Reactions](images/SelectIsocReact.png =x250"Select Isocoulombic Reactions") | ![Select extrapolation methods](images/SelectIsocExtrapolationMethod.png =x250 "Select extrapolation methods") |
|::|::|::|
| _Step 1_ | _Step 2_  | _Step 3_  |

If calculation was successful, view and plot the result by clicking [Show and Plot Results](#) ![Show and Plot Results](images/ShowResults.png "Show and Plot Results"). 

> __Note__  
The result are written in a result_.csv file found in the application folder. 

![Calculated logK values using isocoulombic reactions](images/ResultsPlotInvReact.png =520x"Calculated logK values using isocoulombic reactions")  
_Plot of calculated logK values for an investigated reaction using several isocoulombic reactions and extrapolation methods._

![Calculated logK values from the CSV results file](images/ResultsCsvInvReact.png =520x"Calculated logK values from the CSV results file")  
_Calculated logK values for an investigated reaction using several isocoulombic reactions and extrapolation methods as present in the results\_.csv file. The column names are related to the extrapolation method type (1T, 2T, 3T), the index of the investigated reaction (0, 1, 2, ...)_

__5b.__ Calculated logK values for the generated isocoulombic reactions at different temperature and pressure points, by clicking Tools/[Calculate Properties of Generated Isocoulombic Reactions](#) ![Calculate Properties of Generated Isocoulombic Reactions](images/CalculateRecrod.png "Calculate Properties of Generated Isocoulombic Reactions"). If calculation was successful, view and plot the result by clicking [Show and Plot Results](#) ![Show and Plot Results](images/ShowResults.png "Show and Plot Results").

__6.__ Save the generated isocoulombic reactions, by clicking [Save Reactions Set](#) ![Save.Reactions.Set](images/SaveCurrentRecordIcon.png "Save Reactions Set"). _Step 1_, select the isocoulombic reactions you want to save. _Step 2_, select __one__ extrapolation method which will be set in the saved reactions record. _Step 3_, choose which substance participating in the investigated reaction/s is the reaction defined substance. 

> __Note__     
The substance selected as reaction defined will have a defines edge (link) created between its record and the saved isocoulombic reaction/s. For example, if three isocoulombic reactions are saved for one investigated reaction, three defines edges with different level values will be created between the reaction defined substance and the saved isocoulombic reactions. 

| ![Select Isocoulombic Reactions](images/SelectIsocReact.png =x250"Select Isocoulombic Reactions") | ![Select extrapolation method](images/SelectOneExrapMeth.png =x250 "Select extrapolation methods") | ![Select Substance defined by the reaction](images/SelectDefinedSubst.png =x250"Select Substance defined by the reaction") |
|::|::|::|
| _Step 1_ | _Step 2_  | _Step 3_  |

> __Note__     
When the __Set Extrapolation Methods individually__ is activated in the preferences, when doing logK values calculations or saving generated isocoulombic reactions, you will be asked to select the desired extrapolation method for each reaction individually. 

## Application Menu 
* __File__
  * ![New.Reset](images/NewCreateRecordIcon24.png "New/Reset") __New/Reset__: Resets the Isocoulombic Reactions Generator window. 
  * ![Save.Reactions.Set](images/SaveCurrentRecordIcon.png "Save Reactions Set") __Save Reaction Set__: Save generated isocoulombic reactions in the database as a Reactions Set, for each isocoulombic reaction. Set the Reaction Set __Symbol__, __Comment__ and __Reaction Level__ in the respective fields. 
> __Note__  
The symbol of the saved reaction set will be the value entered in the __Symbol__ field _ the symbol of the isocoulombic reaction    
* __Data__
  * ![Source.Database.Elements](images/SourceElementFilter.png "Source Database Elements") __Select Data Source and Elements__: Opens a wizard for selecting the source database and elements for filtering the data to be used for generating reactions. Select the source database from the available list of source databases. Next, select the elements which the substances and reactions should contain. 
  * ![Select.Investigate.Reactions](images/SelectData.png "Select Investigated Reactions") __Select Investigated Reactions__: Select one or more reactions which are not isocoulombic, but which will be combined into isocoulombic reactions. 
  * ![Select.Model.Reactions](images/SelectMaster.png "Select Model Reactions") __Select Model Reactions__: Select the model reactions, which will be used to generate isocoulombic reactions, by combining them with the selected investigated reactions. 
* __Tools__
  * ![Generate.Isocoulombic.Reactions](images/ReacDCModuleIcon24.png "Generate Isocoulombic Reactions") __Generate Isocoulombic Reactions__: Generate isocoulombic reactions form the selecte reactions to ivestigate and model reactions.
  * ![Calculate Properties of Invesitgated Reactions](images/CalculateRecrod.png "Calculate Properties of Invesitgated Reactions") __Calculate Properties of Reactions to Investigate at TP__: Calculates the logK values of the reactions to investigate at different temperature and pressure points, using the properties of the generated isocoulombic reactions and the one-, two-, or three-term extrapolation method. 
  * ![Calculate Properties of Isocoulombic Reactions](images/CalculateRecrod.png "Calculate Properties of Isocoulombic Reactions") __Calculate Properties of Generated Isocoulombic Reactions at TP__: Calculates the logK values of the generated isocoulombic reactions at different temperature and pressure points, using the one-, two-, or three-term extrapolation method. 
* __Results__
  * ![Show and Plot Results](images/ShowResults.png "Show and Plot Results") __Show table with Results__: Opens the output CSV (Comma Separated Value) file with result and automatically plots the logK values against temperature. 
* __Preferences__
  * __Set extrapolation methods individually__: If set, you will be asked to choose the extrapolation method for each generated isocoulombic reaction when calculating logK values or when saving the reactions. 