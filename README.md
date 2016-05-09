# TCorrPT #

A code for calculating the standard state thermodynamic properties at a given temperature and pressure.

### Simple API example ###

```
#!c++
int main(int argc, char const *argv[])
{
    Database database("database-name.json/xml/yaml");

    Thermo thermo(database);

    ThermoPropertiesSubstance tps;
    ThermoPropertiesReaction tpr;

    tps = thermo.thermoPropertiesSubstance("Formula/Name of Substance", P, T);
    tpr = thermo.thermoPropertiesReaction("Formula/Name of Reaction", P, T);

    return 0;
}
```

### What is this repository for? ###

* Quick summary
* Version
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)

### How do I get set up? ###

* Summary of set up
* Configuration
* Dependencies
* Database configuration
* How to run tests
* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact