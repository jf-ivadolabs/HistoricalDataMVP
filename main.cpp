#include <iostream>
#include <fstream>
#include <vector>
#include </usr/local/include/boost/date_time/gregorian/gregorian.hpp>
#include <string>

using namespace std;
using namespace boost::gregorian; 


const string szPickupKeyword = "Pickup";
const string szDeliveryKeyword = "Delivery";

struct DATE{
    string szDate;    // Date in string format
    int    iMjdDate;  // Date in MJD format (Modified Julian Date)
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
    int earliest_starting_time;
    int earliest_ending_time;

};

/* Instance of a load, specified by a start and end time.  Costs and time is also present they could vary according to when a load is delivered (i.e. rush hour) */
struct SCHEDULED_LOAD {
    LOAD load;
    int start_time;
    int end_time;
};

struct LEG {
    string origin;
    string destination;
    int distance_meters;
    int time_seconds;
};

struct LEGMAP{
    string origin;
    string destination;
    double distance_meters;
    double time_seconds;
};

/* Route structure, which includes a list of scheduled loads (with start and end times), as well as costs, time and kms.  */
struct ROUTES {
    vector<SCHEDULED_LOAD> listOfLoads;
    double total_route_cost;
    double total_route_time;
    double total_route_km;
    int iFirstLoad;
};

// Read double value into *d, return 0 if successful, 1 otherwise. 
int iReadDouble(string szValue, double *d){

       try {
            *d = stod(szValue);
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
int iReadInteger(string szValue, int *i){

       try {
            *i = stoi(szValue);
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
int iReadDate(string szDate, DATE *localDate){
    date d(from_string(szDate));
    localDate->szDate = szDate;
    localDate->iMjdDate = d.modjulian_day();
    return 0;
}

// Read Loads from szFileName and fill load structure. 
int readRyderLoads(vector <LOAD> *loads, string szFileName)
{
    // FIELDS : load_id,delivery_date_id,origin_site_cluster_id,destination_site_cluster_id,movement_type,is_optimizable,cost_cad,weight_kg,distance_km,carrier_type,equipment_type_id
    ifstream file ( szFileName );
    
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
        loads->push_back(load);
        }
    cout<<"Input file "<<szFileName<<" processed"<<endl;
    return 0;
}

// Read legs from szFileName and fill LEGS structure. 
int readRyderLegs(vector<LEG> *legs, string szFileName)
{
    // FIELDS : origin,destination,distance_meters,time_seeconds
    ifstream file ( szFileName );

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
        cout<<"time in second:  "<<value<<endl;
        legs->push_back(leg);
    }
    return 0;

}

// Add load times to LOADS structures based on input read from LEGS
void addLoadTimes(vector <LOAD> *loads, std::map <string, std::map<const string, LEGMAP> > *LegsMap) {
    for(int i=0; i<loads->size(); i++) {
        std::map <string, std::map<const string, LEGMAP> >::iterator itOrigin;
        itOrigin = LegsMap->find(loads->at(i).origin_site_cluster_id);   
        if(itOrigin != LegsMap->end()) {
            std::map<const string, LEGMAP>::iterator itDestination;
            itDestination = itOrigin->second.find(loads->at(i).destination_site_cluster_id);
            if(itDestination != itOrigin->second.end()) {
                loads->at(i).time_seconds = itDestination->second.time_seconds;
            }
            else {
                loads->at(i).is_optimizable = "false";
                cout<<"Unable to find detailed leg info for origin: "<< loads->at(i).origin_site_cluster_id<< " and destination: "<<loads->at(i).destination_site_cluster_id<<endl;
            }
        }
        else {
            loads->at(i).is_optimizable = "false";
            cout<<"Unable to find detailed leg info for origin: "<< loads->at(i).origin_site_cluster_id<< " and destination: "<<loads->at(i).destination_site_cluster_id<<endl;
        }

    }
}

void fillLegsMap(vector<LEG> *legs, std::map <string, std::map<const string, LEGMAP> > *LegsMap)
{

    LEGMAP newLegMap;
    string test = legs->at(0).origin;

    for(int i=0; i<legs->size(); i++) {
        std::map <string, std::map<const string, LEGMAP> >::iterator itOrigin;
        itOrigin = LegsMap->find(legs->at(i).origin);
        newLegMap.origin = legs->at(i).origin;
        newLegMap.destination = legs->at(i).destination;
        newLegMap.time_seconds = legs->at(i).time_seconds;
        newLegMap.distance_meters = legs->at(i).distance_meters;
        
        if(itOrigin != LegsMap->end()) {// If element is already present in map, add the new key
            std::map<const string, LEGMAP> :: iterator itDestination;
            itOrigin->second.insert(std::make_pair(legs->at(i).destination, newLegMap));
        }
        else {
            std::map<const string, LEGMAP> origin_map;
            origin_map.insert(std::make_pair(legs->at(i).destination, newLegMap));
            LegsMap->insert(std::make_pair(legs->at(i).origin, origin_map));
        }
    }

  for (std::map <string, std::map<const string, LEGMAP> >::iterator it=LegsMap->begin(); it!=LegsMap->end(); ++it)
    std::cout << it->first << " => " << it->second.size() << '\n';

}

/* Generate loads at a particular time interval */
void scheduleLoads(int starting_time, int timeIntervals, int endingTime, string szMovementType, int startingIndex, vector <LOAD> *loads, vector <SCHEDULED_LOAD> *schedLoads, vector<ROUTES> *routes) {

    for(int i=startingIndex; i<loads->size(); i++) {
        // Continue if movement type doesn't match
        if(strcmp(loads->at(i).movement_type.c_str(), szMovementType.c_str())) 
            continue;

        int newStartingTime = starting_time + timeIntervals;
        string newMovementType = !strcmp(szMovementType, szDeliveryKeyword) ? szPickupKeyword : szDeliveryKeyword;
        if(newStartingTime<endingTime)
            scheduleLoads(starting_time, timeIntervals, endingTime, newMovementType, i, loads, &schedLoads, routes);
        else {
            // Create new route
            ROUTE newRoute;
            newRoute.iFirstLoad = startingIndex;
        }
    
            // Insert newly created load in all routes which share the same startingIndex

            /* Create instance of scheduled load */
            SCHEDULED_LOAD schedLoad;
            schedLoad.load = loads->at(i);
            schedLoad.start_time = starting_time;
            schedLoad.end_time = schedLoad.start_time + (schedLoad.load.time_seconds/60); // divide by 60 to get time in minutes. 

            newRoute.listOfLoads.push_back(schedLoad);
    




        routes->push_back(newRoute);
    }
}

/* Generate possible routes from historical data stored in loads format */
void generateRoutes(vector<ROUTES> *routes, vector <LOAD> *loads) {

    int starting_time;
    int timeIntervals = 240; // time intervals in-between tasks during the day. 
    int endingTime = 1080;
    vector <SCHEDULED_LOAD> schedLoads; 

    // First try: Every shipment can be done 3 times a day. 
    for(int i=0; i<loads->size(); i++) {

        // Start by doing deliveries
        starting_time = 480; // 8h am
        
        scheduleLoads(starting_time, timeIntervals, endingTime, szDeliveryKeyword, i, loads, &schedLoads, routes);



        // /* Interval 1 */
        // SCHEDULED_LOAD schedLoad;
        // schedLoad.load = loads->at(i);
        // schedLoad.start_time = 480; // 8h am
        // schedLoad.end_time = schedLoad.start_time + (schedLoad.load.time_seconds/60); // 8h am

        // for(int j = i+1; j<loads->size(); j++) {
        //     if(strcmp(loads->at(i).movement_type.c_str(), szDeliveryKeyword.c_str())) 
        //         continue;

        //     SCHEDULED_LOAD schedLoad;
        //     schedLoad.load = loads->at(i);
        //     schedLoad.start_time = 480 +4*60; // 8h am + 4h = 12pm
        //     // TODO: ADD RETURN TIME TO BASE
        //     schedLoad.end_time = schedLoad.start_time + (schedLoad.load.time_seconds/60); // 8h am

        //     for(int k = j+1; k<loads->size(); k++) {
        //         if(strcmp(loads->at(i).movement_type.c_str(), szPickupKeyword.c_str())) 
        //             continue;
            
        //     }

        // }


        // /* Interval 2 */
        // for(j = i+1; j<loads.size(); j++) {

        // }
        // /* Interval 3 */
        // for(j = i+1; j<loads.size(); j++) {

        // }



    }

}

int main () {

    std::map <string, std::map<const string, LEGMAP> > LegsMap; 

    vector<LOAD> loads;
    readRyderLoads(&loads, "ryder_full_loads.txt");
    vector<LEG> legs;
    readRyderLegs(&legs, "ryder_legs.txt");
    fillLegsMap(&legs, &LegsMap);
    addLoadTimes(&loads, &LegsMap);
    vector<ROUTES> routes;
    generateRoutes(&routes, &loads);

    return 0;
}