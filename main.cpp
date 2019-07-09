#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>
#include </usr/local/include/boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <fstream>

using namespace std;
using namespace boost::gregorian; 


const string PICKUP_KEYWORD = "Pickup";
const string DELIVERY_KEYWORD = "Delivery";

// Constants defined for the moment until we get real client hour
const int TIME_INCREMENT = 240;
const int STARTING_TIME = 360; //6AM
const int ENDING_TIME = 1140; //6PM

// Temporarily defimed for 1 instance of the problem
const string PLANNING_START_DAY = "2018-04-11";
const string PLANNING_END_DAY = "2018-04-11";

struct DATE{
    string szDate;    // Date in string format
    int    mjdDate;  // Date in MJD format (Modified Julian Date)
};

struct LOAD {
    /**************************************************************************/
    /*                       CSV FILE INPUT VALUES                            */
    /**************************************************************************/

    int load_id;
    DATE delivery_date_id;
    string origin_site_cluster_id;
    string destination_site_cluster_id;
    string movement_type;
    string is_optimizable;
    double cost_cad;
    double weight_kg;
    string distance_km;
    string carrier_type;
    string equipment_type_id;

    /**************************************************************************/
    /*                 OTHER INPUT/COMPUTED VALUES                            */
    /**************************************************************************/
    int time_seconds;
    int earliest_delivery_time;
    int latest_delivery_time;

};

// Instance of a load, specified by a start and end time.  Costs and time is also present they could vary according to when a load is delivered (i.e. rush hour)
struct SCHEDULED_LOAD {
    LOAD load;
    int start_time;
    int end_time;
};

// A leg is the representation of an arc in-between an origin and a destination. 
struct LEG {
    string origin;
    string destination;
    int distance_meters;
    int time_seconds;
};

// The leg map is used when filling in the travel time in-between nodes. 
struct LEGMAP{
    string origin;
    string destination;
    double distance_meters;
    double time_seconds;
};

// Container for linked list structure of elements of LOAD types
class NODE {
    public:
        LOAD load;
        NODE *next;
        NODE *prev;
        void deletion(NODE**, NODE*);
        NODE * extract(NODE**, NODE*);
        NODE * add_new(NODE**, LOAD);
        void push(NODE**, NODE *);  
        void printList(NODE* );  

};

// Container for linked list structure of elements of SCHEDULED_LOAD types
struct NODE_SCHED_LOAD {
    SCHEDULED_LOAD load;
    NODE_SCHED_LOAD  *next;
    NODE_SCHED_LOAD  *prev;
    NODE_SCHED_LOAD * add_new(NODE_SCHED_LOAD**, SCHEDULED_LOAD);
    NODE_SCHED_LOAD * extract(NODE_SCHED_LOAD**, NODE_SCHED_LOAD*);
};

//Route structure, which includes a list of scheduled loads (with start and end times), as well as costs, time and kms.  */
struct ROUTE {
    NODE_SCHED_LOAD *listOfLoadsHead;
    double total_route_cost;
    double total_route_time;
    double total_route_km;
    int iFirstLoad;
};

// Create new NODE_SCHED_LOAD from SCHEDULED_LOAD and add it to the list. 
NODE_SCHED_LOAD * NODE_SCHED_LOAD::add_new(NODE_SCHED_LOAD **head, SCHEDULED_LOAD load)
{
    if(*head==NULL)
    {
        *head=new NODE_SCHED_LOAD;
        (*head)->load=load;
        (*head)->next=NULL;
        (*head)->prev=NULL;
    }
    else 
    {
        NODE_SCHED_LOAD *n=new NODE_SCHED_LOAD;
        n->load=load;
        n->next=(*head);
        n->prev=NULL;
        (*head)->prev=n;
        (*head)=n;
    }
    return (*head);
}

// Create new NODE_SCHED_LOAD from SCHEDULED_LOAD and add it to the list. 
NODE * NODE::add_new(NODE **head, LOAD load)
{
    if(*head==NULL)
    {
        *head=new NODE;
        (*head)->load=load;
        (*head)->next=NULL;
        (*head)->prev=NULL;
    }
    else 
    {
        NODE *n=new NODE;
        n->load=load;
        n->next=(*head);
        n->prev=NULL;
        (*head)->prev=n;
        (*head)=n;
    }
    return (*head);
}


// Delete node from list
void NODE::deletion(NODE** head, NODE* del)  
{  
    if (*head == NULL || del == NULL)  
        return;  

    if (*head == del)  
        *head = del->next;  
  
    if (del->next != NULL)  
        del->next->prev = del->prev;  
  
    if (del->prev != NULL)  
        del->prev->next = del->next;  

    free(del);  
    return;  
}  

//Remove node from list and return it
NODE * NODE::extract(NODE** head, NODE* del)  
{  
    if (*head == NULL || del == NULL)  
        return NULL;  
  
    if (*head == del)  
        *head = del->next;  
  
    if (del->next != NULL)  
        del->next->prev = del->prev;  

    if (del->prev != NULL)  
        del->prev->next = del->next;  
  
    return del;  
}  

NODE_SCHED_LOAD * NODE_SCHED_LOAD::extract(NODE_SCHED_LOAD** head, NODE_SCHED_LOAD* del)  
{  
    if (*head == NULL || del == NULL)  
        return NULL;  
  
    // special case for head node
    if (*head == del)  
        *head = del->next;  
  
    if (del->next != NULL)  
        del->next->prev = del->prev;  
  
    if (del->prev != NULL)  
        del->prev->next = del->next;  
  
    return del;  
}  

// Insert node at the beginning of the list
void NODE::push(NODE** head, NODE *new_node)  
{  
    new_node->prev = NULL;  
  
    // Link new list to the old. 
    new_node->next = (*head);  
  
    // Link old list to the new node. 
    if ((*head) != NULL)  
        (*head)->prev = new_node;  
  
    // Re-adjust head pointer
    (*head) = new_node;  
}  
  
// Function to print list of load origins
void NODE::printList(NODE* node)  
{  
    while (node != NULL)  
    {  
        cout << node->load.origin_site_cluster_id << " ";  
        node = node->next;  
    }  
    cout<<endl;
}  

// Read double value into *d, return 0 if successful, 1 otherwise. 
int iReadDouble(string value, double *d){

       try {
            *d = stod(value);
                return 0;
        }
        catch (std::invalid_argument const &e)
        {
            std::cout << "Bad input when reading double: std::invalid_argument thrown" << '\n';
        }
        catch (std::out_of_range const &e)
        {
            std::cout << "Double overflow: std::out_of_range thrown" << '\n';
        }
    return 1;   
}

// Read integer into *i, return 0 if successful, 1 otherwise. 
int iReadInteger(string value, int *i){
    try {
        *i = stoi(value);
            return 0;
    }
    catch (std::invalid_argument const &e)
    {
        std::cout << "Bad input when reading integer: std::invalid_argument thrown" << '\n';
    }
    catch (std::out_of_range const &e)
    {
        std::cout << "Integer overflow: std::out_of_range thrown" << '\n';
    }
    return 1;   
}

 // Read date into DATE structure, return 0.
int iReadDate(string value, DATE *localDate){
    date d(from_string(value));
    localDate->szDate = value;
    localDate->mjdDate = d.modjulian_day();
    return 0;
}

// Read Loads from szFileName and fill load structure. 
int readRyderLoads(NODE **loadsHead, string fileName)
{
    // FIELDS : load_id,delivery_date_id,origin_site_cluster_id,destination_site_cluster_id,movement_type,is_optimizable,cost_cad,weight_kg,distance_km,carrier_type,equipment_type_id
    ifstream file ( fileName );
    
    cout<<"Reading ryder loads"<<endl;
    string value;
    DATE loadDate;
    int load_id;
    double cost_cad;
    double weight_kg;
    int distance_km;
    LOAD load;

    if(file.good()) {
        // Read headers
        getline ( file, value, ',' ); //load_id,
        getline ( file, value, ',' ); //delivery_date_id
        getline ( file, value, ',' ); //origin_site_cluster_id
        getline ( file, value, ',' ); //destination_site_cluster_id
        getline ( file, value, ',' ); //movement_type
        getline ( file, value, ',' ); //is_optimizable
        getline ( file, value, ',' ); //cost_cad
        getline ( file, value, ',' ); //weight_kg
        getline ( file, value, ',' ); //distance_km
        getline ( file, value, ',' ); //carrier_type
        getline ( file, value, '\n' ); //equipment_type_id
    }
    // Read rest of info. 
    while ( file.good() )
    {
        // Read DATA
        if(getline ( file, value, ',' )) { //load_id 
            if(iReadInteger(value, &load_id)) return 1;
            load.load_id = load_id;
        }
        else break; // Break when there are no more new load IDs to read. 

        getline ( file, value, ',' ); //delivery_date_id
        if(iReadDate(value, &loadDate)) return 1;
        load.delivery_date_id = loadDate;
        getline ( file, value, ',' ); //origin_site_cluster_id
        load.origin_site_cluster_id = value;
        getline ( file, value, ',' ); //destination_site_cluster_id
        load.destination_site_cluster_id = value;
        getline ( file, value, ',' ); //movement_type
        load.movement_type = value;
        getline ( file, value, ',' ); //is_optimizable
        load.is_optimizable = value;
        getline ( file, value, ',' ); //cost_cad
        if(iReadDouble(value, &cost_cad)) return 1;
        load.cost_cad = cost_cad;
        getline ( file, value, ',' ); //weight_kg
        if(iReadDouble(value, &weight_kg)) return 1;
        load.weight_kg = weight_kg;
        if(iReadInteger(value, &distance_km)) return 1; 
        getline ( file, value, ',' ); //distance_km
        load.distance_km = value;
        getline ( file, value, ',' ); //carrier_type
        load.carrier_type = value;
        getline ( file, value, '\n' ); //equipment_type_id
        load.equipment_type_id = value;

        // NODE *n = new NODE;
        // n->load = load;
        // n->next = NULL;
        // n->prev = NULL;

        // if(*loadsHead == NULL)
        //     *loadsHead = n;
        // else {
        //     n->next = *loadsHead;
        //     (*loadsHead)->prev = n;
        //     *loadsHead = n;
        // }
        (*loadsHead)->add_new(loadsHead, load);

    }
    
    cout<<"Input file "<<fileName<<" processed"<<endl;

    return 0;
}

// Read legs from fileName and fill LEGS structure. 
int readRyderLegs(list<LEG> *legs, string fileName)
{
    // FIELDS : origin,destination,distance_meters,time_seeconds
    ifstream file ( fileName );

    cout<<"Reading ryder legs"<<endl;
    LEG leg;
    string value;
    int distance_meters;
    int time_seconds;

    if(file.good()) {
        // Read headers
        getline ( file, value, ',' ); //origin,
        getline ( file, value, ',' ); //destination
        getline ( file, value, ',' ); //distance_meters
        getline ( file, value, '\n' ); //time_seeconds
    }
    // Read rest of info 
    while ( file.good() )
    {
        // Read DATA
        if(getline ( file, value, ',' )) { //origin
            leg.origin = value;
        }
        else break; // Break when there are no more new legs to read. 

        getline ( file, value, ',' ); //destination
        leg.destination = value;
        getline ( file, value, ',' ); //distance_meters
        if(iReadInteger(value, &distance_meters)) return 1;
        leg.distance_meters = distance_meters;
        getline ( file, value, '\n' ); //time_seconds
        if(iReadInteger(value, &time_seconds)) return 1;        
        leg.time_seconds = time_seconds;
        legs->push_back(leg);
    }
    return 0;

}

// Add load travel times to LOADS structures based on input read from LEGS.
// Also add time needed to travel back to the plant when necessary. 
void addLoadTravelTimes(NODE *loadsHead, std::map <string, std::map<const string, LEGMAP> > *LegsMap) {

//    for(int i=0; i<loads->size(); i++) {
    for (NODE *it = loadsHead; it!=NULL; it = it->next) {

        std::map <string, std::map<const string, LEGMAP> >::iterator itOrigin;
        itOrigin = LegsMap->find(it->load.origin_site_cluster_id);   
        if(itOrigin != LegsMap->end()) {
            std::map<const string, LEGMAP>::iterator itDestination;
            itDestination = itOrigin->second.find(it->load.destination_site_cluster_id);
            if(itDestination != itOrigin->second.end()) {
                it->load.time_seconds = itDestination->second.time_seconds;
                it->load.earliest_delivery_time = STARTING_TIME;
                it->load.latest_delivery_time = ENDING_TIME;
                // TODO: ADD TIME NEEDED TO TRAVEL BACK TO PLAN IF DOING A PICKUP...
            }
            else {
                it->load.is_optimizable = "false";
                cout<<"Unable to find detailed leg info for origin: "<< it->load.origin_site_cluster_id<< " and destination: "<<it->load.destination_site_cluster_id<<endl;
            }
        }
        else {
            it->load.is_optimizable = "false";
            cout<<"Unable to find detailed leg info for origin: "<< it->load.origin_site_cluster_id<< " and destination: "<<it->load.destination_site_cluster_id<<endl;
        }
    }
}

// Leg map is used to map origins/destinations with costs and time from data read in csv file. 
// This data is then passed on to the LOAD structure. 
void fillLegsMap(list <LEG> *legs, std::map <string, std::map<const string, LEGMAP> > *LegsMap)
{
    LEGMAP newLegMap;

    for (std::list<LEG>::iterator it=legs->begin(); it!=legs->end(); ++it) {

        std::map <string, std::map<const string, LEGMAP> >::iterator itOrigin;
        itOrigin = LegsMap->find(it->origin);
        newLegMap.origin = it->origin;
        newLegMap.destination = it->destination;
        newLegMap.time_seconds = it->time_seconds;
        newLegMap.distance_meters = it->distance_meters;
        
        if(itOrigin != LegsMap->end()) {// If element is already present in map, add the new key
            std::map<const string, LEGMAP> :: iterator itDestination;
            itOrigin->second.insert(std::make_pair(it->destination, newLegMap));
        }
        else {
            std::map<const string, LEGMAP> origin_map;
            origin_map.insert(std::make_pair(it->destination, newLegMap));
            LegsMap->insert(std::make_pair(it->origin, origin_map));
        }
    }
}

/* Generate loads at a particular time interval */
void scheduleLoads(int mjdPlanningDay, int starting_time, string szMovementType, NODE **loadsHead, NODE **usedLoadsHead, NODE_SCHED_LOAD **schedLoadsHead, vector<ROUTE> *routes) {

    int cnt=0;
    int newStartingTime = starting_time + TIME_INCREMENT;
    // Alternate between deliveries and pickups
    string newMovementType = !strcmp(szMovementType.c_str(), DELIVERY_KEYWORD.c_str()) ? PICKUP_KEYWORD : DELIVERY_KEYWORD;

    //
    int addedRoute = 0;

    NODE *itNext = *loadsHead;
    for (NODE *it=itNext; it!=NULL; it=itNext) {
        itNext = itNext->next;
        cnt++;
        //cout<<"Load number: " <<cnt<<endl;
        // If we have reached the end of the day, add route with current list of loads
        if(newStartingTime>it->load.latest_delivery_time) {
            if(addedRoute == 0) {
                // Create new route
                ROUTE newRoute;
                //TODO: PROPER COPY CONSTRUCTOR
                newRoute.listOfLoadsHead = *schedLoadsHead;
                //newRoute.total_route_cost = 0;
                //newRoute.total_route_km = 0;
                //newRoute.total_route_time  = 0;
                (routes)->push_back(newRoute);
                addedRoute = 1;
            }
        }
        else
        {
            // Continue if movement type doesn't match
            if(strcmp(it->load.movement_type.c_str(), szMovementType.c_str())) 
                continue;

            // Add current load to "visited" list
            it = it->extract(loadsHead, it);
            (*usedLoadsHead)->push(usedLoadsHead, it);     
 
            int newTime = newStartingTime;
            // For each load, create a route with a different discretized time intervals
            while(newTime<it->load.latest_delivery_time)  {
                //Ensure new task begins after previous task
                if(*schedLoadsHead && newTime < (*schedLoadsHead)->load.end_time) {
                    newTime +=TIME_INCREMENT;
                    continue;
                }
                if((newTime + (it->load.time_seconds/60)) > ENDING_TIME)
                    break;

                /* Create instance of scheduled load */
                SCHEDULED_LOAD schedLoad;
                schedLoad.load = it->load;
                schedLoad.start_time = newTime;
                schedLoad.end_time = schedLoad.start_time + (schedLoad.load.time_seconds/60); // divide by 60 to get time in minutes. 

                // Add new scheduled load to list
                NODE_SCHED_LOAD *newNode = (*schedLoadsHead)->add_new(schedLoadsHead, schedLoad);
                scheduleLoads(mjdPlanningDay, newTime, newMovementType, loadsHead, usedLoadsHead, schedLoadsHead, routes);   

                (*schedLoadsHead)->extract(schedLoadsHead, newNode);      
                newNode = NULL;     
                newTime += TIME_INCREMENT;
            }
            (*usedLoadsHead)->extract(usedLoadsHead, it);
            (*loadsHead)->push(loadsHead, it);
        }
    }
}

/* Generate possible routes from historical data stored in loads format */
void generateRoutes(vector<ROUTE> *routes, NODE **loadsHead) {
    
    NODE *usedLoadsHead = NULL;
    NODE_SCHED_LOAD *schedLoadsHead = NULL;
    DATE mjdPlanningStart;

    iReadDate(PLANNING_START_DAY, &mjdPlanningStart);
    
    // First try: Every shipment can be done 3 times a day. 
//    for(int i=0; i<loads->size(); i++) {
        // Start the day with deliveries (we suppose trucks are preloaded)
        scheduleLoads(mjdPlanningStart.mjdDate, STARTING_TIME, DELIVERY_KEYWORD, loadsHead, &usedLoadsHead, &schedLoadsHead, routes);                
//    }
}

// Remove loads which aren't relevant to the current problem (for which datees don't match our start/end dates)
void filterLoadDates(NODE **loadsHead) {

    DATE mjdPlanningStart;
    DATE mjdPlanningEnd;
 
    iReadDate(PLANNING_START_DAY, &mjdPlanningStart);
    iReadDate(PLANNING_END_DAY, &mjdPlanningEnd);

    cout<<"Removing un-needed loads (outsite of planning period)"<<endl;

    int cnt = 0;
  
    for (NODE *it = *loadsHead ; it != NULL; it = it->next) {
        cnt++;
    }

    cout<<"Loads count before : "<<cnt<<endl;
    cnt=0;
    NODE *itNext = (*loadsHead);
    for (NODE *it = itNext ; it != NULL; it = itNext) {
        itNext = itNext->next;
        if(it->load.delivery_date_id.mjdDate < mjdPlanningStart.mjdDate || it->load.delivery_date_id.mjdDate > mjdPlanningEnd.mjdDate) {
            it->deletion(loadsHead, it);
	        cnt++;   
        }
    
  }
    cout<<"Number of removed loads: "<<cnt<<""<<endl;
    int t;
    cin>>t;
    cnt = 0;
  
    for (NODE *it = *loadsHead ; it != NULL; it = it->next) {
        cnt++;
    }
    cout<<" Remaining loads: "<<cnt<<endl;

    cin>>t;
}

int main () {

    std::map <string, std::map<const string, LEGMAP> > LegsMap; 

    NODE *loadsHead = NULL;
    readRyderLoads(&loadsHead, "ryder_full_loads.txt");
    list<LEG> legs;
    readRyderLegs(&legs, "ryder_legs.txt");
    fillLegsMap(&legs, &LegsMap);
    // Only keep load which fall within planning dates
    filterLoadDates(&loadsHead);
    addLoadTravelTimes(loadsHead, &LegsMap);

    int cnt = 0;
    int hours_cut_off = 6;
    for(NODE *it = loadsHead; it; it = it->next)
    {
        if(it->load.time_seconds/60/60>hours_cut_off){ // remove loads which are longer than 6h. 
            it->deletion(&loadsHead, it);
            cnt++;   
        }
        cout<<"Load time is "<<it->load.time_seconds/60/60<<endl;

    }
    cout<<" Removed : "<<cnt<<" loads because of time >"<<hours_cut_off<<endl;
    vector<ROUTE> routes;\
    generateRoutes(&routes, &loadsHead);

    cnt = 0;
    ofstream routesfile;
    routesfile.open("routes.txt");

    for(std::vector <ROUTE>::iterator it = routes.begin(); it != routes.end(); it++) {
        cnt++;
        routesfile<<"New route: ";
        for(NODE_SCHED_LOAD *schedLoad = it->listOfLoadsHead; schedLoad; schedLoad = schedLoad->next)
            routesfile<<" load_id: "<<schedLoad->load.load.load_id<<" start: "<<schedLoad->load.start_time<<" end: "<<schedLoad->load.end_time<<" ";
        routesfile<<endl;
    }
    cout<<"Number of routes: "<<cnt<<endl;
    return 0;
}
