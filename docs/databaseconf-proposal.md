# Protocols of work  
## with remote ArangoDB on db.thermohub.net and with local ArangoDB on Kubuntu 16.04 LTS desktop     
### DK 17.12.2017 ###

## Protocol 1 ##

### Local installation of ArangoDB ([details here](https://www.arangodb.com/download-major/ubuntu/)) ##

To Add URL to remote repository, open a terminal and execute from the command line:

~~~
~$ curl -O https://download.arangodb.com/arangodb32/xUbuntu_16.04/Release.key

~$ sudo apt-key add - < Release.key

~$ echo 'deb https://download.arangodb.com/arangodb32/xUbuntu_16.04/ /' | sudo tee /etc/apt/sources.list.d/arangodb.list
 
 deb https://download.arangodb.com/arangodb32/xUbuntu_16.04/ /

~$ sudo apt-get update
 
~$ sudo apt-get install arangodb3=3.2.9                                      
~~~

In the process of installation, you will need to answer some questions: 

~~~
......................

Configuring arangodb3
.....................

Please enter the password for the database root user account that should be granted administrative privileges

password for the ArangoDB "root" user: <Enter>

Repeat password for the ArangoDB "root" user: <Enter>

On subsequent updates of this package your database files will have to be upgraded before they can be used with the newer version of ArangoDB. If you select
"yes" here, the files will be automatically upgraded whenever a newer version of ArangoDB is installed in the future. If you select "no" here, the package 
will be rendered 'unstable' and you will need to manually invoke the upgrade procedure on each package upgrade.

Automatically upgrade database files? [yes/no] yes

Choose which storage engine you want to use. Please note that you can't switch storage engines of existing installations, a dump and restore is required therefore. 'auto' will pick the existing one or default to mmfiles.

1. auto  2. mmfiles  3. rocksdb

Database storage engine 3

On subsequent updates of this package upgrades of the database files will happen. If you specify "yes" here, a backup of your database files is created before upgrading the files. The backup will be stored in /var/lib/arangodb-DATE.

Backup database files before upgrading? [yes/no] yes

..........................
~~~

## Protocol 2. ##
### Database and User management ##

The following user concept will be implemented. 

The application clients (ThermoFun, ThermoSol, ThermoKin, ThermoMatch, ThermoFit) have each 
a respective DB user for the remote database and its counterpart for the local (client) database. 
In addition, there will be a remote admin and a local admin to maintain/repair the databases. 

Granting user access rights in HTTP interface ([more](https://docs.arangodb.com/3.2/HTTP/UserManagement/)):
*   Use "rw" to set the database access level to Administrate. 
*   Use "ro" to set the database access level to Access. 
*   Use "none" to set the database access level to No access.

### P2.1. Managing remote database server on https://db.thermohub.net ###

Remote database: secure web interface access via URL entered into web browser:
 
~~~
https://db.thermohub.net
~~~

which resolves to 

~~~
https://db.thermohub.net/_db/_system/_admin/aardvark/index.html#login
~~~

and asks for username and password. To manage databases, collections and users, 
we recommend to login as user "adminrem" with password "Administrator@Remote-ThermoHub-Server". 

Being logged in, choose a database from the drop-down list and follow menus and pages in the aardvark 
web interface for database management ([more](https://docs.arangodb.com/3.2/Manual/Administration/WebInterface/) 
and [even more](https://docs.arangodb.com/3.2/Manual/Administration/WebInterface/Users.html)).

The web interface allows most of management tasks (e.g. creating or deleting databases and collections, 
users, setting user access rights to databases and collections, importing and exporting data from/to 
JSON files, etc.). However, ArangoDB developers recommend using the ArangoDB shell (arangosh) for full 
access to database management functionality, also data upload, download etc. 
([details](https://docs.arangodb.com/3.2/Manual/Administration/Arangosh/)).

To connect via the ArangoDB shell to the remote ThermoFun server from a local machine as root, 
go in a terminal and use a command:

~~~
~$ arangosh --server.endpoint ssl://db.thermohub.net
    Please specify a password:
<Password>
.................

arangosh (ArangoDB 3.2.9 [linux] 64bit, using jemalloc, VPack 0.1.30, RocksDB 5.6.0, ICU 58.1, V8 5.7.492.77, OpenSSL 1.0.2g  1 Mar 2016)
Copyright (c) ArangoDB GmbH
.................

db.thermohub.net@_system>
~~~

To quit ArangoDB shell, type 'exit' or 'quit': 

~~~
db.thermohub.net@_system>exit
~$
~~~

The above default start command of arangosh is equivalent to the following full start:

~~~
~$ arangosh  \
  --server.endpoint ssl://db.thermohub.net \
  --server.username root  \
  --server.password <password> \
  --server.database _system  \
  --server.authentication true
~~~
 
This form should be used for starting arangosh to work with a particular user and database. 
For example, to work with the database "hub_main" as a user "matchrem", use a command 

~~~
 ~$ arangosh  \
  --server.endpoint ssl://db.thermohub.net \
  --server.username match@rem  \
  --server.password ThermoMatch@ThermoHub-Server \
  --server.database hub_main  \
  --server.authentication true
~~~

Passwords are built from the respective application name followed by "@ThermoHub-Server", 
for example "ThermoFun@ThermoHub-Server", "ThermoFits@ThermoHub-Server", etc.

It is highly recommended not to use root access, but the "admin@rem" user instead for 
managing remote ThermoHub databases, with password known by maximum two admin persons.
 
Other project participants and software clients should log in as DB users for the respective databases.   

#### User access rights to remote ThermoHub DB server main (master) databases: #### 

User/database | _system | hub_main | exps_main | fits_main | sys_main
--------------|---------|----------|-----------|-----------|---------  
__adminrem__  |   rw    |   rw     |    rw     |   rw      |   rw
__matchrem__  |   none  |   rw     |    none   |   none    |   ro
__funrem__    |   none  |   ro     |    none   |   none    |   ro
__solrem__    |   none  |   ro     |    none   |   none    |   ro
__kinrem__    |   none  |   ro     |    none   |   none    |   ro
__fitsrem__   |   none  |   ro     |    rw     |   rw      |   rw
__sysrem__    |   none  |   ro     |    ro     |   ro      |   rw    


It is strongly recommended to test new functionality of ThermoMatch, ThermoFits, ThermoSys 
and other clients (especially with rw access) on 'test' databases. 

User access rights to remote ThermoHub DB server test databases. The access rights in the 
table below can be temporarily changed using e.g. web interface, if needed. 


User/database | hub_test | exps_test | fits_test | sys_test | Password
--------------|----------|-----------|-----------|----------|---------  
__adminrem__  |    rw    |    rw     |    rw     |    rw    | Administrator@Remote-ThermoHub-Server 
__matchrem__  |    rw    |    none   |    none   |    ro    | ThermoMatch@Remote-ThermoHub-Server 
__funrem__    |    ro    |    none   |    none   |    ro    | ThermoFun@Remote-ThermoHub-Server 
__solrem__    |    ro    |    none   |    none   |    ro    | ThermoSol@Remote-ThermoHub-Server 
__kinrem__    |    ro    |    none   |    none   |    ro    | ThermoKin@Remote-ThermoHub-Server
__fitsrem__   |    ro    |    rw     |    rw     |    rw    | ThermoFit@Remote-ThermoHub-Server
__sysrem__    |    ro    |    ro     |    ro     |    rw    | ThermoSys@Remote-ThermoHub-Server 


### P2.2. Managing local database via http://localhost:8529 ###

In web browser, local (client) database access via URL http://127.0.0.1:8529 resolves to 

~~~
http://127.0.0.1:8529/_db/_system/_admin/aardvark/index.html#login  
~~~

Initially, the root password for local ArangoDB instance is an empty string (Enter).

To connect via the ArangoDB shell as root to database _system, use a command:

~~~
~$ arangosh --server.endpoint tcp://127.0.0.1:8529 --server.database "_system"
   or
~$ arangosh --server.endpoint tcp://127.0.0.1:8529
   or
~$ arangosh --server.endpoint tcp://localhost:8529
   or simply
~$ arangosh
~~~

Any of these commands results in the following output:

~~~
Please specify a password:
<Enter>

....................

arangosh (ArangoDB 3.2.9 [linux] 64bit, using jemalloc, VPack 0.1.30, RocksDB 5.6.0, ICU 58.1, V8 5.7.492.77, OpenSSL 1.0.2g  1 Mar 2016)
Copyright (c) ArangoDB GmbH

Pretty printing values.
Connected to ArangoDB 'http+tcp://127.0.0.1:8529' version: 3.2.9 [server], database: '_system', username: 'root'

Type 'tutorial' for a tutorial or 'help' to see common examples
127.0.0.1:8529@_system> 
~~~

To leave arangosh, type

~~~
127.0.0.1:8529@_system>quit
~~~
or

~~~
127.0.0.1:8529@_system>exit
~~~
to see:

~~~
Uf wiederluege! 再见！ Na shledanou! Auf Wiedersehen! Bye Bye! Adiau! ¡Hasta luego! Εις το επανιδείν!
להתראות! Arrivederci! Tot ziens! Adjö! Au revoir! さようなら До свидания! Até Breve! خداحافظ! Uz redzēšanos!
~$      
~~~

It is desirable, however, to install a password also for local ArangoDB instance (TBD).

#### Local user access rights to local ThermoHub databases: ####

User/database | _system | hub_work | exps_work | fits_work | sys_work
--------------|---------|----------|-----------|-----------|---------  
__adminloc__  |  rw     |   rw     |    rw     |    rw     |  rw
__matchloc__  |  none   |   rw     |    none   |    none   |  none
__funloc__    |  none   |   rw     |    none   |    none   |  none
__solloc__    |  none   |   rw     |    none   |    none   |  none
__kinloc__    |  none   |   rw     |    none   |    none   |  none
__fitsloc__   |  none   |   rw     |    rw     |    rw     |  rw
__sysloc__    |  none   |   rw     |    rw     |    rw     |  rw


### P2.3. Managing remote and local databases with arangosh ###

Start ArangoDB shell as adminrem (remote DB admin) user

~~~
~$ arangosh  \
   --server.endpoint ssl://db.thermohub.net  \
   --server.username adminrem  \
   --server.password Administrator@Remote-ThermoHub-Server \
   --server.database _system  \
   --server.authentication true
<enter>
..........
                                       _
db.thermohub.net@_system>
~~~

To create a new database "hub_test":

~~~
db.thermohub.net@_system> db._createDatabase("hub_test");
true
~~~

Note: The remote arangosh session may expire in a few minutes of inactivity. If it 
begins to throw errors on DB server access commands, simply quit and restart arangosh.

To create a new local database:

~~~
127.0.0.1:8529@_system> db._createDatabase("hub_work");
true

127.0.0.1:8529@_system>
~~~

To work with a new local database: 

~~~
127.0.0.1:8529@_system> db._useDatabase("hub_work");
true

127.0.0.1:8529@_system>
~~~

### P2.4. Managing remote and local databases via web interface ###

This process is self-explanatory. 

In the web browser, just open the url

~~~
https://db.thermohub.net
~~~

Choose the user "adminrem", enter the password to login, and choose the database "_system" from 
the drop-down list, then click "databases" in the left-hand-side menu.  

To manage databases in a local instance of ArangoDB, open in the browser

~~~
http://localhost:8529
~~~

and choose the user "adminloc", enter the empty password to login, and choose the database 
"_system" from the drop-down list, then click "databases" in the left-hand-side menu.  


### P2.5. Managing remote and local database users with arangosh ###

To create a new remote user "solrem" with password "ThermoSol@Remote-ThermoHub-Server":
Start arangosh as 

~~~
~$ arangosh  \
>   --server.endpoint ssl://db.thermohub.net  \
>   --server.username adminrem  \
>   --server.password Administrator@Remote-ThermoHub-Server \
>   --server.database _system  \
>   --server.authentication true
<enter>
~~~

On the arangosh prompt, execute:

~~~
db.thermohub.net@_system> require("org/arangodb/users").save("solrem", "ThermoSol@Remote-ThermoHub-Server");
{ 
  "user" : "test2", 
  "active" : true, 
  "extra" : { 
  }, 
  "code" : 201 
}
db.thermohub.net@_system>
~~~

To add a new local database user with empty password:

~~~
127.0.0.1:8529@_system> require("org/arangodb/users").save("adminloc", ""); <enter>
{
  "user" : "adminloc",
  "active" : true,
  "extra" : {
  },
  "code" : 201
}

127.0.0.1:8529@_system>
~~~

To change password for the user "adminloc" to "Admin@Local":

~~~
127.0.0.1:8529@_system> require("org/arangodb/users").update( "adminloc", "Admin@Local" ); <enter>
{
  "user" : "adminloc",
  "active" : true,
  "extra" : {
  },
  "code" : 200
}

127.0.0.1:8529@_system>
~~~

To change password for user "adminloc" to the dynamically created password:

~~~
127.0.0.1:8529@_system> passwd = require("internal").genRandomAlphaNumbers(20); <enter>
qPI6iOOhGlPUO4jE6D5S

127.0.0.1:8529@_system> require("org/arangodb/users").update("adminloc", passwd); <enter>
{
  "user" : "adminloc",
  "active" : true,
  "extra" : {
  },
  "code" : 200
}

127.0.0.1:8529@_system>
~~~

### P2.6. Managing remote and local database users via web interface ###

*   This process is self-explanatory. 
*   In the web browser, just open the url

~~~
https://db.thermohub.net
~~~

and choose the user "adminrem", enter the password to login, and choose the database 
"_system" from the drop-down list, then click "users" in the left-hand-side menu. 

To manage users in a local instance of ArangoDB, open in the browser with

~~~
http://localhost:8529
~~~

and choose the user "adminloc", enter the empty password to login, and choose the database 
"_system" from the drop-down list, then click "users" in the left-hand-side menu. 

## Protocol 3 ##

### Local upgrade of ArangoDB ([details here](https://docs.arangodb.com/3.2/Manual/Administration/Upgrading/)) ##

To upgrade an existing ArangoDB database to a newer version of ArangoDB (e.g. 3.2 to 3.3), the following method is recommended:

* Check the CHANGELOG and the list of incompatible changes for API or other changes in the new version of ArangoDB and make sure your applications can deal with them
* Stop the "old" arangod service or binary by a command (on linux)

~~~
$ /etc/init.d/arangod stop
~~~

* Copy the entire "old" data directory to a safe place (that is, a backup)
* Install the new version of ArangoDB and start the server with the --database.auto-upgrade option once:

~~~
$ /etc/init.d/arangod start --database.auto-upgrade
~~~

This might write to the logfile of ArangoDB, so you may want to check the logs for any issues before going on. Stop the service.

* Start the "new" arangod service or binary regularly 

~~~
$ /etc/init.d/arangod start
~~~

and check the logs for any issues. When you're confident everything went well, you may want to check the database directory for any files with the ending .old. These files are created by ArangoDB during upgrades and can be safely removed manually later.

If anything goes wrong during or shortly after the upgrade:

* Stop the "new" arangod service or binary
* Revert to the "old" arangod binary and restore the "old" data directory
* Start the "old" version again

It is not supported to use datafiles created or modified by a newer version of ArangoDB with an older ArangoDB version.

### Installation and use on MacOSX [details here](https://www.arangodb.com/download-major/macosx/) ##

From the download page at arangodb.com, download package for Arangodb CLI, for example

~~~
arangodb3-3.3.0-1.x86_64.dmg (63 MByte)
~~~

Start it and drag the ArangoDB3-CLI.app to /Applications folder. Then start this app and choose RocksDB as backend. 
You should see a terminal window with the following text: 

~~~
ArangoDB server has been started

The database directory is located at
   '/Users/<you>/Library/ArangoDB/var/lib/arangodb3'

The log file is located at
   '/Users/<you>/Library/ArangoDB/var/log/arangodb3/arangod.log'

You can access the server using a browser at 'http://127.0.0.1:8529/'
or start the ArangoDB shell
   '/Applications/ArangoDB3-CLI.app/Contents/MacOS/arangosh'

Switching to log-file now, killing this window will NOT stop the server.

2017-12-23T19:20:24Z [2117] INFO ArangoDB (version 3.3.0 [darwin]) is ready for business. Have fun!
~~~

To use the web interface, open the browser and enter  http://localhost:8529

Then type the user name (initially root) and password (initially empty) and choose the database (initially _system). 


## Appendix. Contents of arangosh help ##

~~~
------------------------------------- Help -------------------------------------
Predefined objects:                                                
  arango:                               ArangoConnection           
  db:                                   ArangoDatabase             
  fm:                                   FoxxManager                
Examples:                                                          
 > db._collections()                    list all collections       
 > db._query(<query>).toArray()         execute an AQL query       
 > db._explain(<query>)                 explain an AQL query       
 > help                                 show help pages            
 > exit                                                            
Note: collection names and statuses may be cached in arangosh.     
To refresh the list of collections and their statuses, issue:      
 > db._collections();                                              
                                                                   
To cancel the current prompt, press CTRL + d.                      


--------------------------- ArangoDatabase (db) help ---------------------------
Administration Functions:                                                 
  _help()                               this help                         
  _flushCache()                         flush and refill collection cache 
                                                                          
Collection Functions:                                                     
  _collections()                        list all collections              
  _collection(<name>)                   get collection by identifier/name 
  _create(<name>, <properties>)         creates a new collection          
  _createEdgeCollection(<name>)         creates a new edge collection     
  _drop(<name>)                         delete a collection               
                                                                          
Document Functions:                                                       
  _document(<id>)                       get document by handle (_id)      
  _replace(<id>, <data>, <overwrite>)   overwrite document                
  _update(<id>, <data>, <overwrite>,    partially update document         
          <keepNull>)                                                     
  _remove(<id>)                         delete document                   
  _exists(<id>)                         checks whether a document exists  
  _truncate()                           delete all documents              
                                                                          
Database Management Functions:                                            
  _createDatabase(<name>)               creates a new database            
  _dropDatabase(<name>)                 drops an existing database        
  _useDatabase(<name>)                  switches into an existing database
  _drop(<name>)                         delete a collection               
  _name()                               name of the current database      
                                                                          
Query / Transaction Functions:                                            
  _executeTransaction(<transaction>)    execute transaction               
  _query(<query>)                       execute AQL query                 
  _createStatement(<data>)              create and return AQL query       
                                                                          
View Functions:                                                           
  _views()                                  list all views                
  _view(<name>)                             get view by name              
  _createView(<name>, <type>, <properties>) creates a new view            

----------------------------- ArangoCollection help -----------------------------
ArangoCollection constructor:                                             
 > col = db.mycoll;                                                       
 > col = db._create("mycoll");                                            
                                                                          
Administration Functions:                                                 
  name()                                collection name                   
  status()                              status of the collection          
  type()                                type of the collection            
  truncate()                            delete all documents              
  properties()                          show collection properties        
  drop()                                delete a collection               
  load()                                load a collection                 
  unload()                              unload a collection               
  rename(<new-name>)                    renames a collection              
  getIndexes()                          return defined indexes            
  refresh()                             refreshes the status and name     
  _help()                               this help                         
                                                                          
Document Functions:                                                       
  count()                               return number of documents        
  save(<data>)                          create document and return handle 
  document(<id>)                        get document by handle (_id or _key)
  replace(<id>, <data>, <overwrite>)    overwrite document                
  update(<id>, <data>, <overwrite>,     partially update document         
         <keepNull>)                                                      
  remove(<id>)                          delete document                   
  exists(<id>)                          checks whether a document exists  
                                                                          
Attributes:                                                               
  _database                             database object                   
  _id                                   collection identifier             

----------------------------- ArangoStatement help -----------------------------
Create an AQL query:                                                    
 > stmt = new ArangoStatement(db, { "query": "FOR..." })                
 > stmt = db._createStatement({ "query": "FOR..." })                    
Set query options:                                                      
 > stmt.setBatchSize(<value>)           set the max. number of results  
                                        to be transferred per roundtrip 
 > stmt.setCount(<value>)               set count flag (return number of
                                        results in "count" attribute)   
Get query options:                                                      
 > stmt.setBatchSize()                  return the max. number of results
                                        to be transferred per roundtrip 
 > stmt.getCount()                      return count flag (return number
                                        of results in "count" attribute)
 > stmt.getQuery()                      return query string             
                                        results in "count" attribute)   
Bind parameters to a query:                                             
 > stmt.bind(<key>, <value>)            bind single variable            
 > stmt.bind(<values>)                  bind multiple variables         
Execute query:                                                          
 > cursor = stmt.execute()              returns a cursor                
Get all results in an array:                                            
 > docs = cursor.toArray()                                              
Or loop over the result set:                                            
 > while (cursor.hasNext()) { print(cursor.next()) }                    

---------------------------- ArangoQueryCursor help ----------------------------
ArangoQueryCursor constructor:                                      
 > cursor = stmt.execute()                                          
Functions:                                                          
  hasNext()                             returns true if there are   
                                        more results to fetch       
  next()                                returns the next document   
  toArray()                             returns all data from the cursor
  _help()                               this help                   
Attributes:                                                         
  _database                             database object             
Example:                                                            
 > stmt = db._createStatement({ "query": "FOR c IN coll RETURN c" })
 > cursor = stmt.execute()                                          
 > documents = cursor.toArray()                                     
 > cursor = stmt.execute()                                          
 > while (cursor.hasNext()) { print(cursor.next())  }               

----------------------------------- More help -----------------------------------
Pager:                                                              
 > stop_pager()                         stop the pager output       
 > start_pager()                        start the pager             
Pretty printing:                                                    
 > stop_pretty_print()                  stop pretty printing        
 > start_pretty_print()                 start pretty printing       
Color output:                                                       
 > stop_color_print()                   stop color printing         
 > start_color_print()                  start color printing        
Print function:                                                     
 > print(x)                             std. print function         
 > print_plain(x)                       print without prettifying   
                                        and without colors          
 > clear()                              clear screen                
~~~





