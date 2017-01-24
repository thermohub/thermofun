#include "DBClient/DBClient.h"
#include "Interfaces/Interface.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    DBClient dbc("./Resources/ThermoFun.ini");

    Database db = dbc.getDatabase(15);

    Thermo th(DBClient("./Resources/ThermoFun.ini").getDatabase(15));

    cout << "Bye World!" << endl;

    return 0;
}

