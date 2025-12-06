#ifndef MARITIME_SYSTEM_H
#define MARITIME_SYSTEM_H
#include <SFML/Graphics.hpp>
#include "data_structures.h"
#include <cmath>
#include <ctime>
#include <sstream>
#include <climits>
using namespace sf;
using namespace std;
struct DateTime // Date time
{
    int day, month, year;
    int hour, minute;
    DateTime() : day(1), month(1), year(2024), hour(0), minute(0) {}
    DateTime(int d, int m, int y, int h, int min)
        : day(d), month(m), year(y), hour(h), minute(min) {}
    bool operator<(const DateTime &other) const
    {
        if (year != other.year)
            return year < other.year;
        if (month != other.month)
            return month < other.month;
        if (day != other.day)
            return day < other.day;
        if (hour != other.hour)
            return hour < other.hour;
        return minute < other.minute;
    }
    bool operator<=(const DateTime &other) const
    {
        return !(other < *this);
    }
    int toMinutes() const
    {
        int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int totalDays = day - 1;
        for (int i = 1; i < month; i++)
        {
            totalDays += daysInMonth[i];
        }
        totalDays += (year - 2024) * 365;
        return (totalDays * 1440) + (hour * 60) + minute;
    }
    int minutesDifference(const DateTime &other) const
    {
        return other.toMinutes() - toMinutes();
    }
    void addMinutes(int mins)
    {
        minute += mins;
        hour += minute / 60;
        minute %= 60;
        day += hour / 24;
        hour %= 24;
    }
    string toString() const
    {
        char buffer[64];
        sprintf(buffer, "%02d/%02d/%04d %02d:%02d", day, month, year, hour, minute);
        return string(buffer);
    }
};
struct Voyage
{ // Voyage info
    int destinationPort;
    MyString company;
    int cost;
    DateTime departure;
    DateTime arrival;
    Voyage *next;
    Voyage() : destinationPort(-1), cost(0), next(nullptr) {}
    int duration() const
    {
        DateTime temp = departure;
        temp.addMinutes(arrival.minutesDifference(departure));
        return departure.minutesDifference(arrival);
    }
    int getDurationMinutes() const
    {
        return duration();
    }
    bool canConnectFrom(const DateTime &previousArrival, int maxLayoverDays) const
    {
        if (previousArrival < departure)
        {
            int layoverMinutes = previousArrival.minutesDifference(departure);
            int layoverDays = (layoverMinutes + 1439) / 1440;
            return layoverDays <= maxLayoverDays;
        }
        return false;
    }
    int getLayoverCost(const DateTime &previousArrival, int portDailyCharge) const
    {
        int layoverMinutes = previousArrival.minutesDifference(departure);
        int layoverDays = (layoverMinutes + 1439) / 1440;
        return layoverDays * portDailyCharge;
    }
};
struct CompanyQueue
{ // Company queue
    MyString companyName;
    Queue<int> shipIds;
    CompanyQueue() {}
    CompanyQueue(const MyString &name) : companyName(name) {}
};
class Port // Port class
{
public:
    MyString name;
    Vector2f position;
    int dailyCharge;
    Voyage *voyages;
    bool isPreferred;
    int dockingQueueSize;
    float queuePulse;
    LinkedList<CompanyQueue> companyQueues;
    Port() : dailyCharge(0), voyages(nullptr), isPreferred(false),
             dockingQueueSize(0), queuePulse(0.0f) {}
    Port(MyString n, Vector2f p) : name(n), position(p), dailyCharge(0),
                                   voyages(nullptr), isPreferred(false), dockingQueueSize(0), queuePulse(0.0f) {}
    ~Port()
    {
        while (voyages)
        {
            Voyage *temp = voyages;
            voyages = voyages->next;
            delete temp;
        }
    }
    void addVoyage(int dest, const MyString &comp, int cst,
                   const DateTime &dep, const DateTime &arr)
    {
        Voyage *newVoyage = new Voyage();
        newVoyage->destinationPort = dest;
        newVoyage->company = comp;
        newVoyage->cost = cst;
        newVoyage->departure = dep;
        newVoyage->arrival = arr;
        newVoyage->next = voyages;
        voyages = newVoyage;
    }
    int getVoyageCount() const
    {
        int count = 0;
        Voyage *current = voyages;
        while (current)
        {
            count++;
            current = current->next;
        }
        return count;
    }
    void addShip(int shipId, const MyString &company)
    {
        bool found = false;
        for (int i = 0; i < companyQueues.size(); i++)
        {
            if (companyQueues.get(i).companyName == company)
            {
                companyQueues.get(i).shipIds.enqueue(shipId);
                found = true;
                break;
            }
        }
        if (!found)
        {
            CompanyQueue newQ(company);
            newQ.shipIds.enqueue(shipId);
            companyQueues.push_back(newQ);
        }
        dockingQueueSize++;
    }
};
class PathNode // Path node
{
public:
    int port;
    long long cost;
    DateTime arrivalTime;
    int previous;
    PathNode() : port(-1), cost(0), previous(-1) {}
    PathNode(int p, long long c, const DateTime &arr, int prev)
        : port(p), cost(c), arrivalTime(arr), previous(prev) {}
};
class Ship // Ship class
{
public:
    int id;
    int currentPort;
    int destinationPort;
    DateTime arrivalTime;
    DateTime departureTime;
    MyString company;
    bool isDocked;
    Ship() : id(0), currentPort(-1), destinationPort(-1), isDocked(false) {}
};
class MaritimeSystem // Main system
{
private:
    static const int MAX_PORTS = 100;
    static const int MAX_SHIPS = 500;
    Port ports[MAX_PORTS];
    int portCount;
    Ship ships[MAX_SHIPS];
    int shipCount;
    LinkedList<int> currentPath;
    LinkedList<int> exploredEdges;
    int startPort, endPort;
    bool showCostPath, showTimePath;
    bool useAStar;
    LinkedList<LinkedList<int> *> allParetoPaths;
    bool showAllPathsMode;
    int hoveredPort;
    int selectedRouteFrom, selectedRouteTo;
    bool routeFocusMode;
    MyString preferredCompany;
    int maxCostPerLeg;
    int maxLayoverDays;
    bool inputActive;
    MyString inputText;
    bool isPreferenceMode;
    Texture mapTexture;
    Sprite mapSprite;
    Font font;
    Clock animationClock;
    MyString notification;
    float notificationTimer;
    Clock notificationClock;

public:
private:
    class BookingAnimation // Animation
    {
    public:
        int currentLeg;
        float progress;
        Vector2f position;
        bool active;
        Clock animationClock;
        int shipId;
        float totalCost;
        int totalTime;
        BookingAnimation() : currentLeg(-1), progress(0.0f), active(false),
                             shipId(-1), totalCost(0), totalTime(0) {}
        void start(int pathStartPort)
        {
            currentLeg = 0;
            progress = 0.0f;
            active = true;
            shipId = 1000 + (rand() % 9000);
            totalCost = 0;
            totalTime = 0;
            position = Vector2f(0, 0);
            animationClock.restart();
        }
        void stop()
        {
            active = false;
            currentLeg = -1;
            progress = 0.0f;
        }
    };
    BookingAnimation bookingAnim;
    bool showBookingAnimation;
    void startBookingAnimation();
    void updateBookingAnimation(float dt);
    void drawBookingAnimation(RenderWindow &window);
    void drawBookingInfo(RenderWindow &window);

private:
    int findCompanyIndex(const MyString &company)
    {
        const char *str = company.c_str();
        int hash = 0;
        for (int i = 0; str[i] != '\0'; i++)
        {
            hash = hash * 31 + str[i];
        }
        return hash;
    }
    void initializePortPositions()
    {
        ports[portCount++] = {MyString("Karachi"), Vector2f(1416, 490)}; // Asia
        ports[portCount++] = {MyString("Dubai"), Vector2f(1366, 480)};
        ports[portCount++] = {MyString("AbuDhabi"), Vector2f(1350, 485)};
        ports[portCount++] = {MyString("Doha"), Vector2f(1330, 470)};
        ports[portCount++] = {MyString("Jeddah"), Vector2f(1298, 500)};
        ports[portCount++] = {MyString("Alexandria"), Vector2f(1258, 430)};
        ports[portCount++] = {MyString("Istanbul"), Vector2f(1254, 384)};
        ports[portCount++] = {MyString("Athens"), Vector2f(1232, 415)};
        ports[portCount++] = {MyString("Mumbai"), Vector2f(1437, 540)};
        ports[portCount++] = {MyString("Colombo"), Vector2f(1460, 600)};
        ports[portCount++] = {MyString("Chittagong"), Vector2f(1500, 520)};
        ports[portCount++] = {MyString("Singapore"), Vector2f(1567, 610)};
        ports[portCount++] = {MyString("Jakarta"), Vector2f(1550, 650)};
        ports[portCount++] = {MyString("HongKong"), Vector2f(1600, 500)};
        ports[portCount++] = {MyString("Shanghai"), Vector2f(1630, 450)};
        ports[portCount++] = {MyString("Busan"), Vector2f(1680, 400)};
        ports[portCount++] = {MyString("Osaka"), Vector2f(1700, 410)};
        ports[portCount++] = {MyString("Tokyo"), Vector2f(1720, 400)};
        ports[portCount++] = {MyString("PortLouis"), Vector2f(1450, 700)}; // Africa
        ports[portCount++] = {MyString("Durban"), Vector2f(1350, 750)};
        ports[portCount++] = {MyString("CapeTown"), Vector2f(1250, 780)};
        ports[portCount++] = {MyString("Lisbon"), Vector2f(1098, 407)}; // Europe
        ports[portCount++] = {MyString("London"), Vector2f(1135, 279)};
        ports[portCount++] = {MyString("Dublin"), Vector2f(1110, 261)};
        ports[portCount++] = {MyString("Rotterdam"), Vector2f(1155, 275)};
        ports[portCount++] = {MyString("Antwerp"), Vector2f(1150, 282)};
        ports[portCount++] = {MyString("Hamburg"), Vector2f(1177, 259)};
        ports[portCount++] = {MyString("Copenhagen"), Vector2f(1187, 238)};
        ports[portCount++] = {MyString("Oslo"), Vector2f(1180, 195)};
        ports[portCount++] = {MyString("Stockholm"), Vector2f(1210, 201)};
        ports[portCount++] = {MyString("Helsinki"), Vector2f(1240, 193)};
        ports[portCount++] = {MyString("StPetersburg"), Vector2f(1261, 195)};
        ports[portCount++] = {MyString("Marseille"), Vector2f(1157, 361)};
        ports[portCount++] = {MyString("NewYork"), Vector2f(833, 394)}; // America
        ports[portCount++] = {MyString("Montreal"), Vector2f(836, 339)};
        ports[portCount++] = {MyString("LosAngeles"), Vector2f(651, 454)};
        ports[portCount++] = {MyString("Vancouver"), Vector2f(630, 304)};
        ports[portCount++] = {MyString("Sydney"), Vector2f(1747, 781)}; // Oceania
        ports[portCount++] = {MyString("Melbourne"), Vector2f(1697, 851)};
    }
    int findPort(const char *name)
    {
        for (int i = 0; i < portCount; i++)
        {
            if (ports[i].name == name)
                return i;
        }
        return -1;
    }
    void loadPortCharges()
    {
        FILE *file = fopen("PortCharges.txt", "r");
        if (!file)
            return;
        char name[64];
        int charge;
        char line[256];
        while (fgets(line, sizeof(line), file))
        {
            if (line[0] == '#' || line[0] == '\n' || line[0] == '\0')
                continue;
            if (sscanf(line, "%s %d", name, &charge) == 2)
            {
                int idx = findPort(name);
                if (idx != -1)
                {
                    ports[idx].dailyCharge = charge;
                }
            }
        }
        fclose(file);
    }
    void loadRoutes()
    { // Load routes
        FILE *file = fopen("Routes.txt", "r");
        if (!file)
        {
            notification = MyString("ERROR: Cannot open Routes.txt");
            notificationTimer = 5.0f;
            notificationClock.restart();
            return;
        }
        char line[256];
        int routeCount = 0;
        while (fgets(line, sizeof(line), file))
        {
            if (line[0] == '\n' || line[0] == '\0' || line[0] == '#')
                continue;
            char from[64], to[64], date[16], dep[16], arr[16], company[64];
            int cost;
            int d, m, y, h1, m1, h2, m2;
            if (sscanf(line, "%s %s %d/%d/%d %d:%d %d:%d %d %s",
                       from, to, &y, &m, &d, &h1, &m1, &h2, &m2, &cost, company) == 11)
            {
                int src = findPort(from);
                int dst = findPort(to);
                if (src != -1 && dst != -1)
                {
                    DateTime departure(d, m, y, h1, m1);
                    DateTime arrival(d, m, y, h2, m2);
                    if (h2 < h1 || (h2 == h1 && m2 < m1))
                    {
                        arrival.day++;
                    }
                    ports[src].addVoyage(dst, MyString(company), cost, departure, arrival);
                    routeCount++;
                }
            }
        }
        fclose(file);
        char buffer[128];
        sprintf(buffer, "Loaded %d routes from %d ports", routeCount, portCount);
        notification = MyString(buffer);
        notificationTimer = 3.0f;
        notificationClock.restart();
        cout << buffer << endl;
    }
    void reconstructIndirectPath(int prev[], int companyIndex[], DateTime arrivalTimes[], bool byCost)
    {
        currentPath.clear();
        if (prev[endPort] == -1)
        {
            notification = MyString("No path found! Try different ports or relax filters.");
            notificationTimer = 3.0f;
            notificationClock.restart();
            return;
        }
        Stack<int> pathStack;
        for (int at = endPort; at != -1; at = prev[at])
        {
            pathStack.push(at);
        }
        if (pathStack.empty() || pathStack.top() != startPort)
        {
            notification = MyString("Path reconstruction failed!");
            notificationTimer = 2.0f;
            notificationClock.restart();
            return;
        }
        while (!pathStack.empty())
        {
            currentPath.push_back(pathStack.top());
            pathStack.pop();
        }
        int totalCost = 0;
        int totalTime = 0;
        int voyageCount = 0;
        int layoverCount = 0;
        vector<string> pathDetails;
        for (int i = 0; i < currentPath.size() - 1; i++)
        {
            int from = currentPath.get(i);
            int to = currentPath.get(i + 1);
            Voyage *takenVoyage = nullptr;
            Voyage *v = ports[from].voyages;
            while (v)
            {
                if (v->destinationPort == to)
                {
                    if (v->arrival.toString() == arrivalTimes[to].toString())
                    {
                        takenVoyage = v;
                        break;
                    }
                }
                v = v->next;
            }
            if (takenVoyage)
            {
                voyageCount++;
                totalCost += takenVoyage->cost;
                totalTime += takenVoyage->getDurationMinutes();
                if (i > 0)
                {
                    int prevPort = currentPath.get(i - 1);
                    Voyage *prevVoyage = nullptr;
                    Voyage *pv = ports[prevPort].voyages;
                    while (pv)
                    {
                        if (pv->destinationPort == from)
                        {
                            if (pv->arrival.toString() == arrivalTimes[from].toString())
                            {
                                prevVoyage = pv;
                                break;
                            }
                        }
                        pv = pv->next;
                    }
                    if (prevVoyage)
                    {
                        int layoverMinutes = takenVoyage->departure.minutesDifference(prevVoyage->arrival);
                        if (layoverMinutes > 0)
                        {
                            layoverCount++;
                            totalTime += layoverMinutes;
                            if (byCost)
                            {
                                int layoverDays = (layoverMinutes + 1439) / 1440;
                                totalCost += layoverDays * ports[from].dailyCharge;
                            }
                            char layoverBuf[128];
                            sprintf(layoverBuf, "Layover at %s: %dh %dm",
                                    ports[from].name.c_str(),
                                    layoverMinutes / 60,
                                    layoverMinutes % 60);
                            pathDetails.push_back(layoverBuf);
                        }
                    }
                }
                char voyageBuf[256];
                sprintf(voyageBuf, "%s → %s (%s)\n     Cost: $%d, Time: %dh %dm, Dep: %s",
                        ports[from].name.c_str(),
                        ports[to].name.c_str(),
                        takenVoyage->company.c_str(),
                        takenVoyage->cost,
                        takenVoyage->getDurationMinutes() / 60,
                        takenVoyage->getDurationMinutes() % 60,
                        takenVoyage->departure.toString().c_str());
                pathDetails.push_back(voyageBuf);
            }
        }
        char summaryBuf[512];
        sprintf(summaryBuf, "%s PATH FOUND!\n"
                            "Route: %s → %s\n"
                            "Segments: %d voyages, %d layovers\n"
                            "Total Cost: $%d\n"
                            "Total Time: %dh %dm",
                byCost ? "CHEAPEST" : "FASTEST",
                ports[startPort].name.c_str(),
                ports[endPort].name.c_str(),
                voyageCount,
                layoverCount,
                totalCost,
                totalTime / 60,
                totalTime % 60);
        notification = MyString(summaryBuf);
        notificationTimer = 5.0f;
        notificationClock.restart();
        cout << "\n=== INDIRECT PATH DETAILS ===" << endl;
        cout << summaryBuf << endl;
        cout << "Detailed itinerary:" << endl;
        for (const string &detail : pathDetails)
        {
            cout << detail << endl;
        }
        cout << "============================\n"
             << endl;
    }
    void findAllPaths()
    {
        if (startPort == -1 || endPort == -1)
        {
            notification = MyString("Please select both start and end ports!");
            notificationTimer = 2.0f;
            notificationClock.restart();
            return;
        }
        cout << "\n=== SEARCHING FOR ALL PARETO-OPTIMAL PATHS ===" << endl;
        cout << "From: " << ports[startPort].name.c_str() << " To: " << ports[endPort].name.c_str() << endl;
        exploredEdges.clear();
        currentPath.clear();
        while (!allParetoPaths.empty())
        {
            delete allParetoPaths.get(0);
            allParetoPaths.remove(0);
        }
        showAllPathsMode = true;
        showCostPath = false;
        showTimePath = false;
        struct SearchNode
        {
            int port;
            long long cost;
            DateTime arrival;
            SearchNode *parent;
            Voyage *voyageFromParent;
            SearchNode(int p, long long c, DateTime a, SearchNode *par, Voyage *v)
                : port(p), cost(c), arrival(a), parent(par), voyageFromParent(v) {}
        };
        LinkedList<SearchNode *> allNodes;
        LinkedList<SearchNode *> visitedStates[MAX_PORTS];
        PriorityQueue<SearchNode *> pq;
        LinkedList<SearchNode *> solutions;
        DateTime startTime(1, 12, 2024, 0, 0);
        SearchNode *startNode = new SearchNode(startPort, 0, startTime, nullptr, nullptr);
        allNodes.push_back(startNode);
        visitedStates[startPort].push_back(startNode);
        pq.push(startNode, 0);
        int nodesExplored = 0;
        while (!pq.empty())
        {
            SearchNode *current = pq.pop();
            int u = current->port;
            if (u == endPort)
            {
                solutions.push_back(current);
                continue;
            }
            nodesExplored++;
            Voyage *v = ports[u].voyages;
            while (v)
            {
                int dest = v->destinationPort;
                if (!preferredCompany.empty() && !(v->company == preferredCompany))
                {
                    v = v->next;
                    continue;
                }
                if (v->cost > maxCostPerLeg)
                {
                    v = v->next;
                    continue;
                }
                if (!v->canConnectFrom(current->arrival, maxLayoverDays))
                {
                    v = v->next;
                    continue;
                }
                int layoverCost = v->getLayoverCost(current->arrival, ports[u].dailyCharge);
                long long legCost = v->cost + layoverCost;
                long long newTotalCost = current->cost + legCost;
                DateTime newArrival = v->arrival;
                bool dominated = false;
                LinkedList<SearchNode *>::Iterator it = visitedStates[dest].begin();
                while (it != visitedStates[dest].end())
                {
                    SearchNode *existing = *it;
                    if (existing->cost <= newTotalCost && existing->arrival <= newArrival)
                    {
                        dominated = true;
                        break;
                    }
                    ++it;
                }
                if (!dominated)
                {
                    SearchNode *newNode = new SearchNode(dest, newTotalCost, newArrival, current, v);
                    allNodes.push_back(newNode);
                    visitedStates[dest].push_back(newNode);
                    pq.push(newNode, (int)newTotalCost);
                    exploredEdges.push_back(u);
                    exploredEdges.push_back(dest);
                }
                v = v->next;
            }
        }
        char buffer[128];
        sprintf(buffer, "Found %d unique paths! Check console.", solutions.size());
        notification = MyString(buffer);
        notificationTimer = 4.0f;
        notificationClock.restart();
        cout << "Found " << solutions.size() << " Pareto-optimal paths:" << endl;
        LinkedList<SearchNode *>::Iterator it = solutions.begin();
        int pathIdx = 1;
        while (it != solutions.end())
        {
            SearchNode *sol = *it;
            int duration = startTime.minutesDifference(sol->arrival);
            cout << "\n[Path " << pathIdx++ << "] Cost: $" << sol->cost
                 << ", Time: " << duration / 60 << "h " << duration % 60 << "m"
                 << ", Arrival: " << sol->arrival.toString().c_str() << endl;
            Stack<int> pathStack;
            SearchNode *curr = sol;
            while (curr)
            {
                pathStack.push(curr->port);
                curr = curr->parent;
            }
            LinkedList<int> *newPath = new LinkedList<int>();
            while (!pathStack.empty())
            {
                int p = pathStack.top();
                cout << ports[p].name.c_str();
                newPath->push_back(p);
                pathStack.pop();
                if (!pathStack.empty())
                    cout << " -> ";
            }
            cout << endl;
            allParetoPaths.push_back(newPath);
            ++it;
        }
        cout << "==============================================\n"
             << endl;
    }
    void dijkstra(bool byCost)
    { // Dijkstra algo
        if (startPort == -1 || endPort == -1)
        {
            notification = MyString("Please select both start and end ports!");
            notificationTimer = 2.0f;
            notificationClock.restart();
            return;
        }
        exploredEdges.clear();
        currentPath.clear();
        struct SearchNode
        {
            int port;
            long long cost;
            DateTime arrival;
            SearchNode *parent;
            Voyage *voyageFromParent;
            SearchNode(int p, long long c, DateTime a, SearchNode *par, Voyage *v)
                : port(p), cost(c), arrival(a), parent(par), voyageFromParent(v) {}
        };
        LinkedList<SearchNode *> allNodes;
        LinkedList<SearchNode *> visitedStates[MAX_PORTS];
        PriorityQueue<SearchNode *> pq;
        DateTime startTime(1, 12, 2024, 0, 0);
        SearchNode *startNode = new SearchNode(startPort, 0, startTime, nullptr, nullptr);
        allNodes.push_back(startNode);
        visitedStates[startPort].push_back(startNode);
        pq.push(startNode, 0);
        char buffer[128];
        sprintf(buffer, "%s finding %s path...",
                useAStar ? "A*" : "Dijkstra",
                byCost ? "cheapest" : "fastest");
        notification = MyString(buffer);
        notificationTimer = 1.5f;
        notificationClock.restart();
        int nodesExplored = 0;
        int connectionsFound = 0;
        SearchNode *finalNode = nullptr;
        while (!pq.empty())
        {
            SearchNode *current = pq.pop();
            int u = current->port;
            if (u == endPort)
            {
                finalNode = current;
                sprintf(buffer, "Path found! Explored %d nodes, %d connections",
                        nodesExplored, connectionsFound);
                notification = MyString(buffer);
                notificationTimer = 2.5f;
                notificationClock.restart();
                break;
            }
            nodesExplored++;
            Voyage *v = ports[u].voyages;
            while (v)
            {
                int dest = v->destinationPort;
                if (!preferredCompany.empty() && !(v->company == preferredCompany))
                {
                    v = v->next;
                    continue;
                }
                if (v->cost > maxCostPerLeg)
                {
                    v = v->next;
                    continue;
                }
                if (!v->canConnectFrom(current->arrival, maxLayoverDays))
                {
                    v = v->next;
                    continue;
                }
                long long legCost = 0;
                if (byCost)
                {
                    int layoverCost = v->getLayoverCost(current->arrival, ports[u].dailyCharge);
                    legCost = v->cost + layoverCost;
                }
                else
                {
                    int layoverMinutes = current->arrival.minutesDifference(v->departure);
                    legCost = v->getDurationMinutes() + layoverMinutes;
                }
                long long newTotalCost = current->cost + legCost;
                DateTime newArrival = v->arrival;
                bool dominated = false;
                LinkedList<SearchNode *>::Iterator it = visitedStates[dest].begin();
                while (it != visitedStates[dest].end())
                {
                    SearchNode *existing = *it;
                    if (existing->cost <= newTotalCost && existing->arrival <= newArrival)
                    {
                        dominated = true;
                        break;
                    }
                    ++it;
                }
                if (!dominated)
                {
                    SearchNode *newNode = new SearchNode(dest, newTotalCost, newArrival, current, v);
                    allNodes.push_back(newNode);
                    visitedStates[dest].push_back(newNode);
                    pq.push(newNode, (int)newTotalCost);
                    exploredEdges.push_back(u);
                    exploredEdges.push_back(dest);
                    connectionsFound++;
                }
                v = v->next;
            }
        }
        if (finalNode)
        {
            Stack<int> pathStack;
            SearchNode *curr = finalNode;
            vector<string> pathDetails;
            int totalCost = 0;
            int totalTime = 0;
            int voyageCount = 0;
            int layoverCount = 0;
            while (curr)
            {
                pathStack.push(curr->port);
                curr = curr->parent;
            }
            while (!pathStack.empty())
            {
                currentPath.push_back(pathStack.top());
                pathStack.pop();
            }
            curr = finalNode;
            while (curr && curr->parent)
            {
                Voyage *v = curr->voyageFromParent;
                SearchNode *parent = curr->parent;
                if (v)
                {
                    voyageCount++;
                    totalCost += v->cost;
                    totalTime += v->getDurationMinutes();
                    int layoverMinutes = parent->arrival.minutesDifference(v->departure);
                    if (layoverMinutes > 0)
                    {
                        layoverCount++;
                        totalTime += layoverMinutes;
                        if (byCost)
                        {
                            int layoverDays = (layoverMinutes + 1439) / 1440;
                            totalCost += layoverDays * ports[parent->port].dailyCharge;
                        }
                        char layoverBuf[128];
                        sprintf(layoverBuf, "Layover at %s: %dh %dm",
                                ports[parent->port].name.c_str(),
                                layoverMinutes / 60,
                                layoverMinutes % 60);
                        pathDetails.push_back(string(layoverBuf));
                    }
                    char voyageBuf[256];
                    sprintf(voyageBuf, "%s → %s (%s)\n     Cost: $%d, Time: %dh %dm, Dep: %s",
                            ports[parent->port].name.c_str(),
                            ports[curr->port].name.c_str(),
                            v->company.c_str(),
                            v->cost,
                            v->getDurationMinutes() / 60,
                            v->getDurationMinutes() % 60,
                            v->departure.toString().c_str());
                    pathDetails.push_back(string(voyageBuf));
                }
                curr = parent;
            }
            reverse(pathDetails.begin(), pathDetails.end());
            char summaryBuf[512];
            sprintf(summaryBuf, "%s PATH FOUND!\n"
                                "Route: %s → %s\n"
                                "Segments: %d voyages, %d layovers\n"
                                "Total Cost: $%d\n"
                                "Total Time: %dh %dm",
                    byCost ? "CHEAPEST" : "FASTEST",
                    ports[startPort].name.c_str(),
                    ports[endPort].name.c_str(),
                    voyageCount,
                    layoverCount,
                    totalCost,
                    totalTime / 60,
                    totalTime % 60);
            notification = MyString(summaryBuf);
            notificationTimer = 5.0f;
            notificationClock.restart();
            cout << "\n=== INDIRECT PATH DETAILS ===" << endl;
            cout << summaryBuf << endl;
            cout << "Detailed itinerary:" << endl;
            for (const string &detail : pathDetails)
            {
                cout << detail << endl;
            }
            cout << "============================\n"
                 << endl;
            if (byCost)
            {
                showCostPath = true;
                showTimePath = false;
            }
            else
            {
                showTimePath = true;
                showCostPath = false;
            }
        }
        else
        {
            notification = MyString("No route found!");
            notificationTimer = 3.0f;
            notificationClock.restart();
        }
        for (int i = 0; i < allNodes.size(); i++)
        {
            delete allNodes.get(i);
        }
    }
    void astar(bool byCost)
    { // A* algo
        if (startPort == -1 || endPort == -1)
        {
            notification = MyString("Please select both start and end ports!");
            notificationTimer = 2.0f;
            notificationClock.restart();
            return;
        }
        long long gScore[MAX_PORTS];
        long long fScore[MAX_PORTS];
        DateTime arrival[MAX_PORTS];
        int prev[MAX_PORTS];
        bool visited[MAX_PORTS] = {false};
        for (int i = 0; i < portCount; i++)
        {
            gScore[i] = fScore[i] = LLONG_MAX;
            prev[i] = -1;
        }
        gScore[startPort] = 0;
        fScore[startPort] = heuristic(startPort, endPort, byCost);
        arrival[startPort] = DateTime(1, 12, 2024, 0, 0);
        PriorityQueue<int> pq;
        pq.push(startPort, fScore[startPort]);
        exploredEdges.clear();
        currentPath.clear();
        char algorithmMsg[128];
        sprintf(algorithmMsg, "A* algorithm running for %s path...",
                byCost ? "cheapest" : "fastest");
        notification = MyString(algorithmMsg);
        notificationTimer = 1.0f;
        notificationClock.restart();
        int nodesExplored = 0;
        while (!pq.empty())
        {
            int u = pq.pop();
            nodesExplored++;
            if (u == endPort)
            {
                char foundMsg[128];
                sprintf(foundMsg, "A* path found! Explored %d nodes", nodesExplored);
                notification = MyString(foundMsg);
                notificationTimer = 2.0f;
                notificationClock.restart();
                break;
            }
            if (visited[u])
                continue;
            visited[u] = true;
            Voyage *v = ports[u].voyages;
            while (v)
            {
                if (!preferredCompany.empty() && !(v->company == preferredCompany))
                {
                    v = v->next;
                    continue;
                }
                if (v->cost > maxCostPerLeg)
                {
                    v = v->next;
                    continue;
                }
                int layoverMinutes = arrival[u].minutesDifference(v->departure);
                if (layoverMinutes < 0)
                {
                    v = v->next;
                    continue;
                }
                int layoverDays = (layoverMinutes + 1439) / 1440;
                if (layoverDays > maxLayoverDays)
                {
                    v = v->next;
                    continue;
                }
                long long layoverCost = layoverDays * ports[u].dailyCharge;
                long long voyageCost = byCost ? (v->cost + layoverCost) : v->getDurationMinutes();
                long long tentativeGScore = gScore[u] + voyageCost;
                if (tentativeGScore < gScore[v->destinationPort])
                {
                    prev[v->destinationPort] = u;
                    gScore[v->destinationPort] = tentativeGScore;
                    fScore[v->destinationPort] = tentativeGScore + heuristic(v->destinationPort, endPort, byCost);
                    arrival[v->destinationPort] = v->arrival;
                    pq.push(v->destinationPort, fScore[v->destinationPort]);
                    exploredEdges.push_back(u);
                    exploredEdges.push_back(v->destinationPort);
                }
                v = v->next;
            }
        }
        reconstructPath(prev);
        if (byCost)
        {
            showCostPath = true;
            showTimePath = false;
        }
        else
        {
            showTimePath = true;
            showCostPath = false;
        }
    }
    long long heuristic(int from, int to, bool byCost)
    {
        float dist = sqrt(pow(ports[to].position.x - ports[from].position.x, 2) +
                          pow(ports[to].position.y - ports[from].position.y, 2));
        if (byCost)
        {
            return dist * 5;
        }
        else
        {
            return dist * 0.5;
        }
    }
    void reconstructPath(int prev[])
    {
        currentPath.clear();
        if (prev[endPort] == -1)
        {
            notification = MyString("No path found between selected ports!");
            notificationTimer = 3.0f;
            notificationClock.restart();
            showCostPath = false;
            showTimePath = false;
            return;
        }
        Stack<int> pathStack;
        for (int at = endPort; at != -1; at = prev[at])
        {
            pathStack.push(at);
        }
        if (pathStack.empty() || pathStack.top() != startPort)
        {
            notification = MyString("Invalid path reconstruction!");
            notificationTimer = 2.0f;
            notificationClock.restart();
            return;
        }
        while (!pathStack.empty())
        {
            currentPath.push_back(pathStack.top());
            pathStack.pop();
        }
        if (currentPath.size() < 2)
        {
            notification = MyString("Path too short!");
            notificationTimer = 2.0f;
            notificationClock.restart();
            return;
        }
        int totalCost = 0;
        int totalTime = 0;
        int segments = currentPath.size() - 1;
        for (int i = 0; i < segments; i++)
        {
            int from = currentPath.get(i);
            int to = currentPath.get(i + 1);
            Voyage *v = ports[from].voyages;
            while (v)
            {
                if (v->destinationPort == to)
                {
                    totalCost += v->cost;
                    totalTime += v->getDurationMinutes();
                    break;
                }
                v = v->next;
            }
        }
        char buffer[256];
        sprintf(buffer, "Path found: %d segments\nTotal Cost: $%d\nTotal Time: %dh %dm",
                segments, totalCost, totalTime / 60, totalTime % 60);
        notification = MyString(buffer);
        notificationTimer = 4.0f;
        notificationClock.restart();
        cout << "\n=== PATH FOUND ===" << endl;
        cout << "Path: ";
        for (int i = 0; i < currentPath.size(); i++)
        {
            cout << ports[currentPath.get(i)].name.c_str();
            if (i < currentPath.size() - 1)
                cout << " → ";
        }
        cout << "\nSegments: " << segments << endl;
        cout << "Total Cost: $" << totalCost << endl;
        cout << "Total Time: " << totalTime / 60 << "h " << totalTime % 60 << "m" << endl;
        cout << "=================\n"
             << endl;
    }
    void drawPortGeometry(RenderWindow &window, int idx)
    {
        const Port &port = ports[idx];
        if (port.dockingQueueSize > 0)
        {
            float radius = 25.0f + port.dockingQueueSize * 3.0f;
            float pulse = 0.5f + 0.5f * sin(animationClock.getElapsedTime().asSeconds() * 2.0f + port.queuePulse);
            CircleShape queueCircle(radius);
            queueCircle.setPosition(port.position - Vector2f(radius, radius));
            queueCircle.setFillColor(Color::Transparent);
            queueCircle.setOutlineColor(Color(255, 200, 0, static_cast<Uint8>(180 * pulse)));
            queueCircle.setOutlineThickness(2.0f);
            queueCircle.setPointCount(32);
            window.draw(queueCircle);
        }
        CircleShape portCircle(10.0f);
        portCircle.setPosition(port.position - Vector2f(10, 10));
        if (idx == startPort)
        {
            portCircle.setFillColor(Color::Green);
        }
        else if (idx == endPort)
        {
            portCircle.setFillColor(Color(255, 165, 0));
        }
        else if (idx == hoveredPort)
        {
            portCircle.setFillColor(Color::Yellow);
        }
        else if (port.isPreferred)
        {
            portCircle.setFillColor(Color::Magenta);
        }
        else
        {
            portCircle.setFillColor(Color(70, 130, 180));
        }
        portCircle.setOutlineThickness(2.0f);
        portCircle.setOutlineColor(Color::White);
        window.draw(portCircle);
    }
    void drawPortInfo(RenderWindow &window, int idx)
    {
        const Port &port = ports[idx];
        Text nameText(port.name.c_str(), font, 14);
        nameText.setFillColor(Color::White);
        nameText.setStyle(Text::Bold);
        nameText.setPosition(port.position + Vector2f(15, -25));
        if (port.dockingQueueSize > 0)
        {
            Text queueText(to_string(port.dockingQueueSize), font, 14);
            queueText.setFillColor(Color(255, 200, 0));
            queueText.setStyle(Text::Bold);
            queueText.setPosition(port.position + Vector2f(15, -10));
            window.draw(queueText);
        }
        if (idx == hoveredPort)
        {
            string tooltip = string(port.name.c_str()) + "\n";
            tooltip += "Charge: $" + to_string(port.dailyCharge) + "/day\n";
            tooltip += "Routes: " + to_string(port.getVoyageCount()) + "\n";
            Voyage *v = port.voyages;
            int count = 0;
            while (v && count < 3)
            {
                tooltip += "  " + string(ports[v->destinationPort].name.c_str()) +
                           " (" + v->company.c_str() + ")\n";
                v = v->next;
                count++;
            }
            if (port.getVoyageCount() > 3)
                tooltip += "...\n";
            if (port.dockingQueueSize > 0)
            {
                tooltip += "Queue: " + to_string(port.dockingQueueSize) + " ships";
            }
            RectangleShape tooltipBg(Vector2f(220, 120 + min(3, port.getVoyageCount()) * 20));
            tooltipBg.setFillColor(Color(0, 0, 0, 230));
            tooltipBg.setOutlineColor(Color::White);
            tooltipBg.setOutlineThickness(1);
            tooltipBg.setPosition(port.position + Vector2f(15, -15));
            window.draw(tooltipBg);
            Text tooltipText(tooltip, font, 12);
            tooltipText.setFillColor(Color::White);
            tooltipText.setPosition(port.position + Vector2f(25, -5));
            window.draw(tooltipText);
        }
    }
    void drawRouteLine(RenderWindow &window, int from, int to, const Voyage &voyage)
    {
        Vector2f start = ports[from].position;
        Vector2f end = ports[to].position;
        bool isFocused = (from == selectedRouteFrom && to == selectedRouteTo);
        bool isInPath = false;
        for (int i = 0; i < currentPath.size() - 1; i++)
        {
            if ((currentPath.get(i) == from && currentPath.get(i + 1) == to) ||
                (currentPath.get(i) == to && currentPath.get(i + 1) == from))
            {
                isInPath = true;
                break;
            }
        }
        Color lineColor(100, 150, 255, 150);
        float thickness = 2.0f + (voyage.cost / 10000.0f);
        if (isFocused)
        {
            lineColor = Color::Yellow;
            thickness = 5.0f;
        }
        else if (isInPath && showCostPath)
        {
            lineColor = Color::Yellow;
            thickness = 4.0f;
        }
        else if (isInPath && showTimePath)
        {
            lineColor = Color::Green;
            thickness = 4.0f;
        }
        else if (from == hoveredPort || to == hoveredPort)
        {
            lineColor = Color::Cyan;
            thickness = 3.0f;
        }
        Vertex line[] = {
            Vertex(start, lineColor),
            Vertex(end, lineColor)};
        for (int i = 0; i < 2; i++)
        {
            line[i].color = lineColor;
        }
        window.draw(line, 2, Lines);
    }
    void drawRouteInfo(RenderWindow &window, int from, int to, const Voyage &voyage)
    {
        Vector2f start = ports[from].position;
        Vector2f end = ports[to].position;
        if (voyage.cost > 0)
        {
            Vector2f mid = (start + end) * 0.5f;
            char costStr[32];
            sprintf(costStr, "$%d", voyage.cost);
            Text costText(costStr, font, 12);
            costText.setFillColor(Color::White);
            costText.setStyle(Text::Bold);
            FloatRect bounds = costText.getLocalBounds();
            costText.setOrigin(bounds.width / 2, bounds.height / 2);
            costText.setPosition(mid);
            RectangleShape bg(Vector2f(bounds.width + 10, bounds.height + 5));
            bg.setFillColor(Color(0, 0, 0, 150));
            bg.setPosition(mid - Vector2f(bounds.width / 2 + 5, bounds.height / 2 + 3));
        }
        if (from == selectedRouteFrom && to == selectedRouteTo)
        {
            Vector2f infoPos = (start + end) * 0.5f + Vector2f(20, -50);
            string details = string(voyage.company.c_str()) + "\n";
            details += "Dep: " + voyage.departure.toString() + "\n";
            details += "Arr: " + voyage.arrival.toString() + "\n";
            details += "Cost: $" + to_string(voyage.cost) + "\n";
            details += "Dur: " + to_string(voyage.getDurationMinutes() / 60) + "h" +
                       to_string(voyage.getDurationMinutes() % 60) + "m";
            RectangleShape infoBg(Vector2f(180, 110));
            infoBg.setFillColor(Color(0, 0, 0, 220));
            infoBg.setOutlineColor(Color::Yellow);
            infoBg.setOutlineThickness(1);
            infoBg.setPosition(infoPos);
            window.draw(infoBg);
            Text infoText(details.c_str(), font, 12);
            infoText.setFillColor(Color::Yellow);
            infoText.setPosition(infoPos + Vector2f(10, 10));
            window.draw(infoText);
        }
    }
    void drawPathLine(RenderWindow &window)
    {
        if (currentPath.size() < 2)
            return;
        if (!showCostPath && !showTimePath)
            return;
        Color pathColor = showCostPath ? Color::Yellow : Color::Green;
        float pulse = 0.6f + 0.4f * sin(animationClock.getElapsedTime().asSeconds() * 2.0f);
        pathColor.a = static_cast<Uint8>(180 + 75 * pulse);
        for (int i = 0; i < currentPath.size() - 1; i++)
        {
            int from = currentPath.get(i);
            int to = currentPath.get(i + 1);
            Vector2f start = ports[from].position;
            Vector2f end = ports[to].position;
            Vertex line[] = {
                Vertex(start, pathColor),
                Vertex(end, pathColor)};
            bool isDirect = false;
            Voyage *v = ports[from].voyages;
            while (v)
            {
                if (v->destinationPort == to)
                {
                    isDirect = true;
                    break;
                }
                v = v->next;
            }
            Vector2f direction = end - start;
            float length = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (length > 0)
                direction /= length;
            if (isDirect)
            {
                window.draw(line, 2, Lines);
            }
            else
            {
                float dashLength = 15.0f;
                float gapLength = 8.0f;
                float traveled = 0.0f;
                while (traveled < length)
                {
                    float segLength = min(dashLength, length - traveled);
                    Vector2f segStart = start + direction * traveled;
                    Vector2f segEnd = segStart + direction * segLength;
                    Vertex dash[] = {
                        Vertex(segStart, pathColor),
                        Vertex(segEnd, pathColor)};
                    window.draw(dash, 2, Lines);
                    traveled += segLength + gapLength;
                }
            }
            if (length > 30.0f)
            {
                Vector2f arrowBase = end - direction * 15.0f;
                Vector2f perpendicular(-direction.y, direction.x);
                Vertex arrow[] = {
                    Vertex(end, pathColor),
                    Vertex(arrowBase + perpendicular * 6.0f, pathColor),
                    Vertex(end, pathColor),
                    Vertex(arrowBase - perpendicular * 6.0f, pathColor)};
                window.draw(arrow, 4, Lines);
            }
        }
    }
    void drawPathInfo(RenderWindow &window)
    {
        if (currentPath.size() < 2)
            return;
        if (!showCostPath && !showTimePath)
            return;
        for (int i = 0; i < currentPath.size() - 1; i++)
        {
            int from = currentPath.get(i);
            Vector2f start = ports[from].position;
            if (i > 0 && i < currentPath.size() - 1)
            {
                CircleShape transferCircle(8.0f);
                transferCircle.setPosition(start - Vector2f(8, 8));
                transferCircle.setFillColor(Color::Transparent);
                transferCircle.setOutlineColor(Color::Cyan);
                transferCircle.setOutlineThickness(2.0f);
                transferCircle.setPointCount(6);
                window.draw(transferCircle);
                Text transferText("↻", font, 14);
                transferText.setFillColor(Color::Cyan);
                transferText.setPosition(start - Vector2f(5, 10));
                window.draw(transferText);
            }
        }
    }
    void drawPath(RenderWindow &window)
    {
        if (currentPath.size() < 2)
            return;
        if (!showCostPath && !showTimePath)
            return;
        drawPathLine(window);
        drawPathInfo(window);
        drawPathDetailsPanel(window);
    }
    void drawPathDetailsPanel(RenderWindow &window)
    {
        if (currentPath.size() < 2)
            return;
        string itinerary = "ITINERARY:\n";
        int legNumber = 1;
        for (int i = 0; i < currentPath.size() - 1; i++)
        {
            int from = currentPath.get(i);
            int to = currentPath.get(i + 1);
            Voyage *v = ports[from].voyages;
            string voyageDetails = "";
            while (v)
            {
                if (v->destinationPort == to)
                {
                    char buf[128];
                    sprintf(buf, "%d. %s → %s\n   %s | $%d | %dh%dm\n",
                            legNumber++,
                            ports[from].name.c_str(),
                            ports[to].name.c_str(),
                            v->company.c_str(),
                            v->cost,
                            v->getDurationMinutes() / 60,
                            v->getDurationMinutes() % 60);
                    voyageDetails = buf;
                    break;
                }
                v = v->next;
            }
            if (voyageDetails.empty())
            {
                char buf[128];
                sprintf(buf, "%d. %s → %s\n   [TRANSFER REQUIRED]\n",
                        legNumber++,
                        ports[from].name.c_str(),
                        ports[to].name.c_str());
                voyageDetails = buf;
            }
            itinerary += voyageDetails;
            if (i < currentPath.size() - 2)
            {
                itinerary += " Layover at port\n";
            }
        }
        RectangleShape panel(Vector2f(400, min(300.0f, 50.0f + legNumber * 40.0f)));
        panel.setFillColor(Color(0, 0, 0, 220));
        panel.setOutlineColor(showCostPath ? Color::Yellow : Color::Green);
        panel.setOutlineThickness(3);
        panel.setPosition(390, 840);
        window.draw(panel);
        Text itineraryText(itinerary, font, 14);
        itineraryText.setFillColor(Color::White);
        itineraryText.setPosition(400, 880);
        window.draw(itineraryText);
        string pathType = showCostPath ? "CHEAPEST PATH" : "FASTEST PATH";
        Text typeText(pathType, font, 18);
        typeText.setFillColor(showCostPath ? Color::Yellow : Color::Green);
        typeText.setStyle(Text::Bold);
        typeText.setPosition(480, 850);
        window.draw(typeText);
    }
    void drawPathSummary(RenderWindow &window)
    {
        if (currentPath.size() < 2)
            return;
        int totalCost = 0;
        int totalTime = 0;
        int segments = currentPath.size() - 1;
        for (int i = 0; i < segments; i++)
        {
            int from = currentPath.get(i);
            int to = currentPath.get(i + 1);
            Voyage *v = ports[from].voyages;
            while (v)
            {
                if (v->destinationPort == to)
                {
                    totalCost += v->cost;
                    totalTime += v->getDurationMinutes();
                    break;
                }
                v = v->next;
            }
        }
        string summary;
        if (showCostPath)
        {
            summary = "CHEAPEST PATH FOUND:\n";
        }
        else if (showTimePath)
        {
            summary = "FASTEST PATH FOUND:\n";
        }
        else
        {
            summary = "CURRENT PATH:\n";
        }
        summary += "Route: ";
        for (int i = 0; i < min(currentPath.size(), 4); i++)
        {
            summary += ports[currentPath.get(i)].name.c_str();
            if (i < min(currentPath.size(), 4) - 1)
                summary += " → ";
        }
        if (currentPath.size() > 4)
            summary += " → ...";
        summary += "\nSegments: " + to_string(segments);
        summary += "\nTotal Cost: $" + to_string(totalCost);
        summary += "\nTotal Time: " + to_string(totalTime / 60) + "h " + to_string(totalTime % 60) + "m";
        RectangleShape panel(Vector2f(400, 140));
        panel.setFillColor(Color(0, 0, 0, 200));
        panel.setOutlineColor(showCostPath ? Color::Yellow : Color::Green);
        panel.setOutlineThickness(3);
        panel.setPosition(1520, 20);
        Text summaryText(summary, font, 16);
        summaryText.setFillColor(showCostPath ? Color::Yellow : Color::Green);
        summaryText.setPosition(1530, 30);
        window.draw(summaryText);
        window.draw(panel);
    }
    void drawUI(RenderWindow &window)
    {
        RectangleShape panel(Vector2f(360, 1080));
        panel.setFillColor(Color(20, 30, 50, 220));
        panel.setOutlineColor(Color(0, 150, 200));
        panel.setOutlineThickness(3);
        panel.setPosition(0, 0);
        window.draw(panel);
        Text title("OCEANROUTE NAV", font, 32);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        title.setPosition(45, 30);
        window.draw(title);
        Text subtitle("Maritime Navigation Optimizer", font, 16);
        subtitle.setFillColor(Color(200, 230, 255));
        subtitle.setPosition(50, 70);
        window.draw(subtitle);
        float yPos = 120;
        RectangleShape selectionBox(Vector2f(320, 100));
        selectionBox.setFillColor(Color(30, 40, 60, 200));
        selectionBox.setOutlineColor(Color(0, 200, 150));
        selectionBox.setOutlineThickness(2);
        selectionBox.setPosition(20, yPos);
        window.draw(selectionBox);
        Text selectionTitle("CURRENT SELECTION", font, 14);
        selectionTitle.setFillColor(Color(0, 255, 200));
        selectionTitle.setStyle(Text::Bold);
        selectionTitle.setPosition(30, yPos + 10);
        window.draw(selectionTitle);
        if (startPort != -1)
        {
            Text fromText("FROM: " + string(ports[startPort].name.c_str()), font, 16);
            fromText.setFillColor(Color::Green);
            fromText.setPosition(40, yPos + 35);
            window.draw(fromText);
        }
        if (endPort != -1)
        {
            Text toText("TO: " + string(ports[endPort].name.c_str()), font, 16);
            toText.setFillColor(Color(255, 165, 0));
            toText.setPosition(40, yPos + 60);
            window.draw(toText);
        }
        yPos += 120;
        RectangleShape algoBox(Vector2f(320, 50));
        algoBox.setFillColor(Color(40, 30, 60, 200));
        algoBox.setOutlineColor(useAStar ? Color(255, 200, 0) : Color(0, 200, 255));
        algoBox.setOutlineThickness(2);
        algoBox.setPosition(20, yPos);
        window.draw(algoBox);
        string algoName = useAStar ? "A* Algorithm" : "Dijkstra's Algorithm";
        Text algoText(algoName, font, 16);
        algoText.setFillColor(useAStar ? Color(255, 220, 100) : Color(100, 220, 255));
        algoText.setStyle(Text::Bold);
        algoText.setPosition(30, yPos + 15);
        window.draw(algoText);
        yPos += 70;
        if (bookingAnim.active)
        {
            RectangleShape animBox(Vector2f(320, 60));
            animBox.setFillColor(Color(60, 40, 30, 200));
            animBox.setOutlineColor(Color(255, 215, 0));
            animBox.setOutlineThickness(2);
            animBox.setPosition(20, yPos);
            window.draw(animBox);
            string animStatus = "SHIP #" + to_string(bookingAnim.shipId) + " ACTIVE";
            Text animText(animStatus, font, 16);
            animText.setFillColor(Color(255, 215, 0));
            animText.setStyle(Text::Bold);
            animText.setPosition(30, yPos + 20);
            window.draw(animText);
            yPos += 80;
        }
        RectangleShape prefBox(Vector2f(320, 80));
        prefBox.setFillColor(Color(30, 40, 30, 200));
        prefBox.setOutlineColor(Color(100, 255, 100));
        prefBox.setOutlineThickness(2);
        prefBox.setPosition(20, yPos);
        window.draw(prefBox);
        Text prefTitle("PREFERENCES", font, 14);
        prefTitle.setFillColor(Color(100, 255, 100));
        prefTitle.setStyle(Text::Bold);
        prefTitle.setPosition(30, yPos + 10);
        window.draw(prefTitle);
        string prefStr = "Company: ";
        prefStr += preferredCompany.empty() ? "Any" : preferredCompany.c_str();
        prefStr += "\nMax Cost: $" + to_string(maxCostPerLeg);
        Text prefText(prefStr, font, 12);
        prefText.setFillColor(Color::White);
        prefText.setPosition(40, yPos + 35);
        window.draw(prefText);
        yPos += 100;
        RectangleShape controlBox(Vector2f(320, 450));
        controlBox.setFillColor(Color(40, 30, 40, 200));
        controlBox.setOutlineColor(Color(200, 100, 255));
        controlBox.setOutlineThickness(2);
        controlBox.setPosition(20, yPos);
        window.draw(controlBox);
        Text controlTitle("CONTROLS", font, 18);
        controlTitle.setFillColor(Color(200, 100, 255));
        controlTitle.setStyle(Text::Bold);
        controlTitle.setPosition(30, yPos + 10);
        window.draw(controlTitle);
        vector<string> controls = {
            "[C] - Find Cheapest Path",
            "[F] - Find Fastest Path",
            "[V] - View All Paths",
            "[A] - Toggle A*/Dijkstra",
            "[B] - Book Route (Instant)",
            "[N] - Book Route (Animated)",
            "[P] - Set Preferences",
            "[Q] - Clear Filters",
            "[R] - Reset All",
            "[ESC] - Close",
            "",
            "Mouse Controls:",
            "  Left Click - Select Ports",
            "  Click Route - View Details",
            "  Ctrl+Click - Toggle Preferred",
            "  Hover - See Port Info",
            "",
            "",
            "",
            "Booking Animation:",
            "  Ship follows selected path",
            "  Port queues update in real-time",
            "  Progress bar shows journey %"};
        float controlY = yPos + 45;
        for (const string &ctrl : controls)
        {
            Text ctrlText(ctrl, font, 14);
            ctrlText.setFillColor(Color::White);
            ctrlText.setPosition(40, controlY);
            window.draw(ctrlText);
            controlY += 25;
        }
        yPos += 470;
        if (inputActive)
        {
            RectangleShape inputBox(Vector2f(320, 60));
            inputBox.setFillColor(Color(50, 40, 30, 220));
            inputBox.setOutlineColor(Color(255, 200, 0));
            inputBox.setOutlineThickness(3);
            inputBox.setPosition(20, yPos);
            window.draw(inputBox);
            string prompt = isPreferenceMode ? "Preferences: " : "Search: ";
            Text promptText(prompt, font, 14);
            promptText.setFillColor(Color(255, 220, 150));
            promptText.setPosition(30, yPos + 10);
            window.draw(promptText);
            Text inputTextDisplay(inputText.c_str() + string("_"), font, 16);
            inputTextDisplay.setFillColor(Color::White);
            inputTextDisplay.setStyle(Text::Bold);
            inputTextDisplay.setPosition(30, yPos + 35);
            window.draw(inputTextDisplay);
        }
        RectangleShape legendPanel(Vector2f(290, 450));
        legendPanel.setFillColor(Color(20, 30, 50, 220));
        legendPanel.setOutlineColor(Color(255, 150, 0));
        legendPanel.setOutlineThickness(3);
        legendPanel.setPosition(1625, 5);
        window.draw(legendPanel);
        Text legendTitle("LEGEND", font, 24);
        legendTitle.setFillColor(Color(255, 180, 0));
        legendTitle.setStyle(Text::Bold);
        legendTitle.setPosition(1680, 10);
        window.draw(legendTitle);
        vector<pair<Color, string>> legendItems = {
            {Color::Green, "Start Port"},
            {Color(255, 165, 0), "End Port"},
            {Color::Yellow, "Hovered Port"},
            {Color::Magenta, "Preferred Port"},
            {Color(100, 150, 255), "Normal Route"},
            {Color::Yellow, "Cheapest Path"},
            {Color::Green, "Fastest Path"},
            {Color(255, 215, 0), "Moving Ship"},
            {Color(255, 200, 0), "Docking Queue"},
            {Color::Cyan, "Hovered Route"}};
        float legendY = 70;
        for (const auto &item : legendItems)
        {
            RectangleShape colorBox(Vector2f(12, 12));
            colorBox.setFillColor(item.first);
            colorBox.setPosition(1633, legendY);
            window.draw(colorBox);
            Text itemText(item.second, font, 14);
            itemText.setFillColor(Color::White);
            itemText.setPosition(1653, legendY - 3);
            window.draw(itemText);
            legendY += 30;
        }
        int totalRoutes = 0;
        int activeShips = bookingAnim.active ? 1 : 0;
        for (int i = 0; i < portCount; i++)
        {
            totalRoutes += ports[i].getVoyageCount();
        }
        string stats = "Ports: " + to_string(portCount) + "\n";
        stats += "Routes: " + to_string(totalRoutes) + "\n";
        stats += "Active Ships: " + to_string(activeShips) + "\n";
        stats += "Queued Ships: ";
        int totalQueued = 0;
        for (int i = 0; i < portCount; i++)
        {
            totalQueued += ports[i].dockingQueueSize;
        }
        stats += to_string(totalQueued);
        Text statsText(stats, font, 14);
        statsText.setFillColor(Color(180, 220, 255));
        statsText.setPosition(1630, 370);
        window.draw(statsText);
        if (notificationTimer > 0)
        {
            float elapsed = notificationClock.getElapsedTime().asSeconds();
            float alpha = min(1.0f, notificationTimer - elapsed) * 255.0f;
            RectangleShape notifBg(Vector2f(500, 110));
            notifBg.setFillColor(Color(0, 0, 0, static_cast<Uint8>(alpha * 0.8f)));
            notifBg.setOutlineColor(Color(255, 255, 255, static_cast<Uint8>(alpha)));
            notifBg.setOutlineThickness(2);
            notifBg.setPosition(700, 30);
            window.draw(notifBg);
            Text notifText(notification.c_str(), font, 16);
            notifText.setFillColor(Color(255, 255, 255, static_cast<Uint8>(alpha)));
            notifText.setStyle(Text::Bold);
            FloatRect bounds = notifText.getLocalBounds();
            notifText.setOrigin(bounds.width / 2, bounds.height / 2);
            notifText.setPosition(960, 80);
            window.draw(notifText);
        }
    }

public:
    MaritimeSystem() : portCount(0), shipCount(0), startPort(-1), endPort(-1),
                       hoveredPort(-1), selectedRouteFrom(-1), selectedRouteTo(-1),
                       showCostPath(false), showTimePath(false), useAStar(false),
                       routeFocusMode(false), maxCostPerLeg(1000000),
                       maxLayoverDays(14), inputActive(false), isPreferenceMode(false),
                       notificationTimer(0.0f), showBookingAnimation(false)
    {
        bookingAnim.currentLeg = -1;
        bookingAnim.progress = 0.0f;
        bookingAnim.active = false;
        bookingAnim.shipId = -1;
        bookingAnim.totalCost = 0;
        bookingAnim.totalTime = 0;
        bookingAnim.position = Vector2f(0, 0);
        bool mapLoaded = mapTexture.loadFromFile("./texture/map.jpg");
        if (!mapLoaded)
        {
        }
        else
        {
            mapSprite.setTexture(mapTexture);
            mapSprite.setScale(1600.0f / mapTexture.getSize().x,
                               1100.0f / mapTexture.getSize().y);
            mapSprite.setPosition(360.f, 0.f);
            std::cout << "Map loaded: " << mapTexture.getSize().x << "x"
                      << mapTexture.getSize().y << " → scaled to 2000x1100\n";
        }
        mapSprite.setColor(Color(255, 255, 255, 220));
        if (!font.loadFromFile("./font/ArialMdmItl.ttf"))
        {
        }
        initializePortPositions();
        loadPortCharges();
        loadRoutes();
        srand(static_cast<unsigned int>(time(nullptr)));
        notification = MyString("System Ready - Press [N] for booking animation");
        notificationTimer = 3.0f;
        notificationClock.restart();
        animationClock.restart();
    }
    void handleClick(const Vector2f &mousePos, bool rightClick)
    {
        if (rightClick)
        {
            routeFocusMode = false;
            selectedRouteFrom = selectedRouteTo = -1;
            return;
        }
        hoveredPort = -1;
        for (int i = 0; i < portCount; i++)
        {
            float dist = sqrt(pow(mousePos.x - ports[i].position.x, 2) +
                              pow(mousePos.y - ports[i].position.y, 2));
            if (dist < 15.0f)
            {
                hoveredPort = i;
                if (Keyboard::isKeyPressed(Keyboard::LControl))
                {
                    ports[i].isPreferred = !ports[i].isPreferred;
                    notification = MyString(ports[i].isPreferred ? "Marked as preferred" : "Removed from preferred");
                    notificationTimer = 2.0f;
                    notificationClock.restart();
                }
                else
                {
                    if (startPort == -1)
                    {
                        startPort = i;
                        notification = MyString("Start port selected");
                    }
                    else if (endPort == -1 && i != startPort)
                    {
                        endPort = i;
                        notification = MyString("End port selected");
                    }
                    else
                    {
                        startPort = i;
                        endPort = -1;
                        notification = MyString("New start port selected");
                    }
                    notificationTimer = 2.0f;
                    notificationClock.restart();
                }
                return;
            }
        }
        selectedRouteFrom = selectedRouteTo = -1;
        float closestDist = 20.0f;
        for (int i = 0; i < portCount; i++)
        {
            Voyage *v = ports[i].voyages;
            while (v)
            {
                Vector2f p1 = ports[i].position;
                Vector2f p2 = ports[v->destinationPort].position;
                Vector2f line = p2 - p1;
                float lineLength = sqrt(line.x * line.x + line.y * line.y);
                if (lineLength < 0.001f)
                    continue;
                Vector2f lineDir = line / lineLength;
                Vector2f toPoint = mousePos - p1;
                float dotProduct = toPoint.x * lineDir.x + toPoint.y * lineDir.y;
                float projection = max(0.0f, min(lineLength, dotProduct));
                Vector2f closest = p1 + lineDir * projection;
                Vector2f diff = mousePos - closest;
                float dist = sqrt(diff.x * diff.x + diff.y * diff.y);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    selectedRouteFrom = i;
                    selectedRouteTo = v->destinationPort;
                    routeFocusMode = true;
                }
                v = v->next;
            }
        }
        if (selectedRouteFrom != -1)
        {
            notification = MyString("Route selected - see details");
            notificationTimer = 2.0f;
            notificationClock.restart();
        }
    }
    void handleHover(const Vector2f &mousePos)
    {
        hoveredPort = -1;
        for (int i = 0; i < portCount; i++)
        {
            float dist = sqrt(pow(mousePos.x - ports[i].position.x, 2) +
                              pow(mousePos.y - ports[i].position.y, 2));
            if (dist < 15.0f)
            {
                hoveredPort = i;
                break;
            }
        }
    }
    void handleKey(Keyboard::Key key)
    {
        switch (key)
        {
        case Keyboard::V:
            findAllPaths();
            break;
        case Keyboard::C:
            if (startPort != -1 && endPort != -1)
            {
                if (useAStar)
                    astar(true);
                else
                    dijkstra(true);
                showCostPath = true;
                showTimePath = false;
                showAllPathsMode = false;
                if (bookingAnim.active)
                {
                    bookingAnim.stop();
                    notification = MyString("Finding new path... animation stopped");
                    notificationTimer = 2.0f;
                }
                if (currentPath.size() > 2)
                {
                    notification = MyString("Multi-leg path found! Check itinerary panel.");
                    notificationTimer = 3.0f;
                }
            }
            else
            {
                notification = MyString("Select START port (click), then END port");
                notificationTimer = 2.5f;
            }
            notificationClock.restart();
            break;
        case Keyboard::F:
            if (startPort != -1 && endPort != -1)
            {
                if (useAStar)
                    astar(false);
                else
                    dijkstra(false);
                showCostPath = false;
                showTimePath = true;
                showAllPathsMode = false;
                if (bookingAnim.active)
                {
                    bookingAnim.stop();
                    notification = MyString("Animation stopped - New path calculated");
                    notificationTimer = 2.0f;
                    notificationClock.restart();
                }
            }
            else
            {
                notification = MyString("Please select both start and end ports first");
                notificationTimer = 2.0f;
                notificationClock.restart();
            }
            break;
        case Keyboard::A:
            useAStar = !useAStar;
            notification = MyString(useAStar ? "Switched to A* algorithm" : "Switched to Dijkstra algorithm");
            notificationTimer = 2.0f;
            notificationClock.restart();
            break;
        case Keyboard::B:
            if (currentPath.size() >= 2)
            {
                for (int i = 0; i < currentPath.size(); i++)
                {
                    int portIdx = currentPath.get(i);
                    ports[portIdx].dockingQueueSize++;
                    ports[portIdx].queuePulse = static_cast<float>(i) * 0.8f;
                }
                int totalCost = 0;
                int totalTime = 0;
                for (int i = 0; i < currentPath.size() - 1; i++)
                {
                    int from = currentPath.get(i);
                    int to = currentPath.get(i + 1);
                    Voyage *v = ports[from].voyages;
                    while (v)
                    {
                        if (v->destinationPort == to)
                        {
                            totalCost += v->cost;
                            totalTime += v->getDurationMinutes();
                            break;
                        }
                        v = v->next;
                    }
                }
                char buffer[256];
                sprintf(buffer, "Route booked instantly!\n%d ships added to port queues\nTotal Cost: $%d\nTotal Time: %dh %dm",
                        currentPath.size(), totalCost, totalTime / 60, totalTime % 60);
                notification = MyString(buffer);
                notificationTimer = 4.0f;
                notificationClock.restart();
            }
            else
            {
                notification = MyString("No path to book! Find a path first (C/F keys)");
                notificationTimer = 2.0f;
                notificationClock.restart();
            }
            break;
        case Keyboard::N:
            if (currentPath.size() >= 2)
            {
                if (!bookingAnim.active)
                {
                    startBookingAnimation();
                }
                else
                {
                    char buffer[128];
                    sprintf(buffer, "Animation already running!\nShip #%d at leg %d/%d",
                            bookingAnim.shipId,
                            bookingAnim.currentLeg + 1,
                            currentPath.size() - 1);
                    notification = MyString(buffer);
                    notificationTimer = 3.0f;
                    notificationClock.restart();
                }
            }
            else
            {
                notification = MyString("No path to animate! Find a path first (C/F keys)");
                notificationTimer = 2.0f;
                notificationClock.restart();
            }
            break;
        case Keyboard::P:
            inputActive = true;
            isPreferenceMode = true;
            inputText = MyString("");
            notification = MyString("Enter preferences (e.g., 'company:Maersk' or 'maxcost:5000')");
            notificationTimer = 3.0f;
            notificationClock.restart();
            break;
        case Keyboard::Q:
            preferredCompany = MyString("");
            maxCostPerLeg = 1000000;
            maxLayoverDays = 14;
            notification = MyString("All filters cleared to default values");
            notificationTimer = 2.0f;
            notificationClock.restart();
            break;
        case Keyboard::R:
            startPort = endPort = -1;
            currentPath.clear();
            exploredEdges.clear();
            showCostPath = showTimePath = false;
            routeFocusMode = false;
            selectedRouteFrom = selectedRouteTo = -1;
            preferredCompany = MyString("");
            maxCostPerLeg = 1000000;
            maxLayoverDays = 14;
            if (bookingAnim.active)
            {
                bookingAnim.stop();
            }
            for (int i = 0; i < portCount; i++)
            {
                ports[i].dockingQueueSize = 0;
                ports[i].isPreferred = false;
                ports[i].queuePulse = 0.0f;
            }
            notification = MyString("System completely reset");
            notificationTimer = 2.5f;
            notificationClock.restart();
            break;
        case Keyboard::Enter:
            if (inputActive && !inputText.empty())
            {
                if (isPreferenceMode)
                {
                    string str = inputText.c_str();
                    while (!str.empty() && isspace(str[0]))
                        str.erase(0, 1);
                    while (!str.empty() && isspace(str.back()))
                        str.pop_back();
                    if (str.find("company:") == 0)
                    {
                        preferredCompany = MyString(str.substr(8).c_str());
                        char buffer[128];
                        sprintf(buffer, "Company filter set to: %s", preferredCompany.c_str());
                        notification = MyString(buffer);
                    }
                    else if (str.find("maxcost:") == 0)
                    {
                        try
                        {
                            maxCostPerLeg = stoi(str.substr(8));
                            char buffer[128];
                            sprintf(buffer, "Max cost per leg set to: $%d", maxCostPerLeg);
                            notification = MyString(buffer);
                        }
                        catch (...)
                        {
                            notification = MyString("Invalid cost value! Use: maxcost:5000");
                        }
                    }
                    else if (str.find("maxlayover:") == 0)
                    {
                        try
                        {
                            maxLayoverDays = stoi(str.substr(11));
                            char buffer[128];
                            sprintf(buffer, "Max layover days set to: %d", maxLayoverDays);
                            notification = MyString(buffer);
                        }
                        catch (...)
                        {
                            notification = MyString("Invalid layover value! Use: maxlayover:3");
                        }
                    }
                    else
                    {
                        notification = MyString("Unknown preference! Use: company:X, maxcost:X, maxlayover:X");
                    }
                }
                inputActive = false;
                notificationTimer = 2.5f;
                notificationClock.restart();
            }
            break;
        case Keyboard::Escape:
            inputActive = false;
            routeFocusMode = false;
            selectedRouteFrom = selectedRouteTo = -1;
            if (bookingAnim.active && Keyboard::isKeyPressed(Keyboard::LControl))
            {
                bookingAnim.stop();
                notification = MyString("Animation stopped (Ctrl+Esc)");
                notificationTimer = 2.0f;
                notificationClock.restart();
            }
            break;
        case Keyboard::Space:
            if (bookingAnim.active)
            {
                static bool paused = false;
                paused = !paused;
                if (paused)
                {
                    static Time pausedTime;
                    pausedTime = bookingAnim.animationClock.getElapsedTime();
                    notification = MyString("Animation paused");
                }
                else
                {
                    notification = MyString("Animation resumed");
                }
                notificationTimer = 1.5f;
                notificationClock.restart();
            }
            break;
        case Keyboard::S:
            if (bookingAnim.active && Keyboard::isKeyPressed(Keyboard::LShift))
            {
                notification = MyString("Animation speed increased (Shift+S)");
                notificationTimer = 1.5f;
                notificationClock.restart();
            }
            else if (bookingAnim.active && Keyboard::isKeyPressed(Keyboard::LControl))
            {
                notification = MyString("Animation speed decreased (Ctrl+S)");
                notificationTimer = 1.5f;
                notificationClock.restart();
            }
            break;
        case Keyboard::T:
            if (showCostPath || showTimePath)
            {
                showCostPath = !showCostPath;
                showTimePath = !showTimePath;
                notification = MyString(showCostPath ? "Showing cost path" : "Showing time path");
                notificationTimer = 2.0f;
                notificationClock.restart();
            }
            break;
        case Keyboard::H:
            notification = MyString("Help: C=Cheapest, F=Fastest, B=Book, N=Animate, R=Reset, ESC=Exit");
            notificationTimer = 4.0f;
            notificationClock.restart();
            break;
        default:
            if (key >= Keyboard::Num0 && key <= Keyboard::Num9)
            {
                int number = key - Keyboard::Num0;
                switch (number)
                {
                case 1:
                    startPort = findPort("Karachi");
                    endPort = findPort("Singapore");
                    if (startPort != -1 && endPort != -1)
                    {
                        notification = MyString("Quick test: Karachi → Singapore");
                        notificationTimer = 2.0f;
                    }
                    break;
                case 2:
                    startPort = findPort("Dubai");
                    endPort = findPort("HongKong");
                    if (startPort != -1 && endPort != -1)
                    {
                        notification = MyString("Quick test: Dubai → HongKong");
                        notificationTimer = 2.0f;
                    }
                    break;
                case 3:
                    startPort = findPort("London");
                    endPort = findPort("NewYork");
                    if (startPort != -1 && endPort != -1)
                    {
                        notification = MyString("Quick test: London → NewYork");
                        notificationTimer = 2.0f;
                    }
                    break;
                case 4:
                    startPort = findPort("Tokyo");
                    endPort = findPort("Sydney");
                    if (startPort != -1 && endPort != -1)
                    {
                        notification = MyString("Quick test: Tokyo → Sydney");
                        notificationTimer = 2.0f;
                    }
                    break;
                }
                notificationClock.restart();
            }
            break;
        }
    }
    void handleText(Uint32 unicode)
    {
        if (!inputActive)
            return;
        if (unicode == 8)
        {
            if (!inputText.empty())
            {
                string str = inputText.c_str();
                str.pop_back();
                inputText = MyString(str.c_str());
            }
        }
        else if (unicode >= 32 && unicode < 128)
        {
            string str = inputText.c_str();
            str += static_cast<char>(unicode);
            inputText = MyString(str.c_str());
        }
    }
    void update(float dt)
    {
        if (notificationTimer > 0)
        {
            notificationTimer -= dt;
        }
        if (bookingAnim.active)
        {
            updateBookingAnimation(dt);
        }
        for (int i = 0; i < portCount; i++)
        {
            if (ports[i].dockingQueueSize > 0)
            {
                ports[i].queuePulse += dt * 0.5f;
                if (ports[i].queuePulse > 6.28f)
                {
                    ports[i].queuePulse -= 6.28f;
                }
            }
        }
    }
    void draw(RenderWindow &window)
    {
        window.draw(mapSprite);
        for (int i = 0; i < portCount; i++)
        {
            Voyage *v = ports[i].voyages;
            while (v)
            {
                drawRouteLine(window, i, v->destinationPort, *v);
                v = v->next;
            }
        }
        if (!exploredEdges.empty())
        {
            float alpha = 100.0f + 100.0f * sin(animationClock.getElapsedTime().asSeconds() * 2.0f);
            Color exploreColor(100, 200, 255, static_cast<Uint8>(alpha));
            for (int i = 0; i < exploredEdges.size(); i += 2)
            {
                int from = exploredEdges.get(i);
                int to = exploredEdges.get(i + 1);
                Vertex line[] = {
                    Vertex(ports[from].position, exploreColor),
                    Vertex(ports[to].position, exploreColor)};
                window.draw(line, 2, Lines);
                CircleShape exploredDot(3.0f);
                exploredDot.setFillColor(Color(100, 200, 255, 180));
                exploredDot.setPosition(ports[from].position - Vector2f(3, 3));
                window.draw(exploredDot);
                exploredDot.setPosition(ports[to].position - Vector2f(3, 3));
                window.draw(exploredDot);
            }
        }
        drawPathLine(window);
        drawAllFoundPaths(window);
        for (int i = 0; i < portCount; i++)
        {
            drawPortGeometry(window, i);
        }
        if (bookingAnim.active)
        {
            drawBookingAnimation(window);
        }
        for (int i = 0; i < portCount; i++)
        {
            Voyage *v = ports[i].voyages;
            while (v)
            {
                drawRouteInfo(window, i, v->destinationPort, *v);
                v = v->next;
            }
        }
        drawPathInfo(window);
        drawPathDetailsPanel(window);
        for (int i = 0; i < portCount; i++)
        {
            drawPortInfo(window, i);
        }
        if (bookingAnim.active)
        {
            drawBookingInfo(window);
        }
        drawUI(window);
    }
    void drawAllFoundPaths(RenderWindow &window)
    {
        if (!showAllPathsMode || allParetoPaths.empty())
            return;
        Color colors[] = {
            Color::Cyan,
            Color::Magenta,
            Color::Yellow,
            Color::Green,
            Color::White};
        float pulse = 0.5f + 0.5f * sin(animationClock.getElapsedTime().asSeconds() * 3.0f);
        for (int p = 0; p < allParetoPaths.size(); p++)
        {
            LinkedList<int> *path = allParetoPaths.get(p);
            Color pathColor = colors[p % 5];
            pathColor.a = static_cast<Uint8>(150 + 100 * pulse);
            Vector2f offset(p * 2.0f - 4.0f, p * 2.0f - 4.0f);
            for (int i = 0; i < path->size() - 1; i++)
            {
                int from = path->get(i);
                int to = path->get(i + 1);
                Vector2f start = ports[from].position + offset;
                Vector2f end = ports[to].position + offset;
                Vector2f direction = end - start;
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                direction /= length;
                Vector2f normal(-direction.y, direction.x);
                float thickness = 3.0f;
                Vertex line[] = {
                    Vertex(start - normal * thickness, pathColor),
                    Vertex(start + normal * thickness, pathColor),
                    Vertex(end + normal * thickness, pathColor),
                    Vertex(end - normal * thickness, pathColor)};
                window.draw(line, 4, Quads);
                if (length > 30.0f)
                {
                    Vector2f arrowBase = end - direction * 15.0f;
                    Vertex arrow[] = {
                        Vertex(end, pathColor),
                        Vertex(arrowBase + normal * 6.0f, pathColor),
                        Vertex(end, pathColor),
                        Vertex(arrowBase - normal * 6.0f, pathColor)};
                    window.draw(arrow, 4, Lines);
                }
            }
        }
        RectangleShape legendBg(Vector2f(200, 30 + allParetoPaths.size() * 25));
        legendBg.setFillColor(Color(0, 0, 0, 200));
        legendBg.setOutlineColor(Color::White);
        legendBg.setOutlineThickness(1);
        legendBg.setPosition(1500, 500);
        window.draw(legendBg);
        Text title("FOUND PATHS", font, 14);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        title.setPosition(1510, 510);
        window.draw(title);
        for (int p = 0; p < allParetoPaths.size(); p++)
        {
            Color c = colors[p % 5];
            RectangleShape box(Vector2f(15, 15));
            box.setFillColor(c);
            box.setPosition(1510, 540 + p * 25);
            window.draw(box);
            string label = "Path " + to_string(p + 1);
            Text labelText(label, font, 12);
            labelText.setFillColor(Color::White);
            labelText.setPosition(1535, 540 + p * 25);
            window.draw(labelText);
        }
    }
    bool isInputActive() const { return inputActive; }
    void runTests();
};
inline void MaritimeSystem::startBookingAnimation()
{
    if (currentPath.size() < 2)
    {
        notification = MyString("No path to animate!");
        notificationTimer = 2.0f;
        notificationClock.restart();
        return;
    }
    bookingAnim.start(startPort);
    showBookingAnimation = true;
    for (int i = 0; i < portCount; i++)
    {
        ports[i].dockingQueueSize = 0;
    }
    bookingAnim.totalCost = 0;
    bookingAnim.totalTime = 0;
    for (int i = 0; i < currentPath.size() - 1; i++)
    {
        int from = currentPath.get(i);
        int to = currentPath.get(i + 1);
        Voyage *v = ports[from].voyages;
        while (v)
        {
            if (v->destinationPort == to)
            {
                bookingAnim.totalCost += v->cost;
                bookingAnim.totalTime += v->getDurationMinutes();
                break;
            }
            v = v->next;
        }
    }
    char buffer[128];
    sprintf(buffer, "Booking animation started! Ship #%d", bookingAnim.shipId);
    notification = MyString(buffer);
    notificationTimer = 3.0f;
    notificationClock.restart();
}
inline void MaritimeSystem::updateBookingAnimation(float dt)
{
    if (!bookingAnim.active)
        return;
    float animationSpeed = 0.4f;
    if (bookingAnim.currentLeg >= 0 && bookingAnim.currentLeg < currentPath.size() - 1)
    {
        bookingAnim.progress += dt * animationSpeed;
        if (bookingAnim.progress >= 1.0f)
        {
            int arrivedPort = currentPath.get(bookingAnim.currentLeg + 1);
            ports[arrivedPort].dockingQueueSize++;
            ports[arrivedPort].queuePulse = bookingAnim.currentLeg * 0.8f;
            bookingAnim.currentLeg++;
            bookingAnim.progress = 0.0f;
            if (bookingAnim.currentLeg >= currentPath.size() - 1)
            {
                bookingAnim.active = false;
                showBookingAnimation = false;
                int finalPort = currentPath.get(currentPath.size() - 1);
                ports[finalPort].dockingQueueSize += 2;
                char buffer[128];
                sprintf(buffer, "Route completed! Ship #%d docked at %s",
                        bookingAnim.shipId, ports[finalPort].name.c_str());
                notification = MyString(buffer);
                notificationTimer = 4.0f;
                notificationClock.restart();
                return;
            }
        }
        int from = currentPath.get(bookingAnim.currentLeg);
        int to = currentPath.get(bookingAnim.currentLeg + 1);
        Vector2f start = ports[from].position;
        Vector2f end = ports[to].position;
        bookingAnim.position = start + (end - start) * bookingAnim.progress;
    }
}
inline void MaritimeSystem::drawBookingAnimation(RenderWindow &window)
{
    if (!bookingAnim.active)
        return;
    float elapsed = bookingAnim.animationClock.getElapsedTime().asSeconds();
    float pulse = 0.7f + 0.3f * sin(elapsed * 4.0f);
    Vector2f shipPos = bookingAnim.position;
    CircleShape hull(12.0f, 6);
    hull.setPosition(shipPos - Vector2f(12, 12));
    hull.setFillColor(Color(255, 215, 0, 240));
    hull.setOutlineColor(Color(139, 69, 19, 255));
    hull.setOutlineThickness(2.0f);
    hull.setRotation(45.0f);
    window.draw(hull);
    RectangleShape cabin(Vector2f(10.0f, 8.0f));
    cabin.setPosition(shipPos - Vector2f(5, 18));
    cabin.setFillColor(Color(70, 130, 180, 255));
    cabin.setOutlineColor(Color::White);
    cabin.setOutlineThickness(1.0f);
    window.draw(cabin);
    RectangleShape mast(Vector2f(3.0f, 20.0f));
    mast.setPosition(shipPos - Vector2f(1.5f, 38));
    mast.setFillColor(Color(160, 82, 45));
    window.draw(mast);
    ConvexShape flag(3);
    flag.setPoint(0, Vector2f(0, 0));
    flag.setPoint(1, Vector2f(15, 7));
    flag.setPoint(2, Vector2f(0, 14));
    flag.setPosition(shipPos.x + 1.5f, shipPos.y - 38);
    flag.setFillColor(Color::Red);
    window.draw(flag);
    Text shipText("S" + to_string(bookingAnim.shipId), font, 10);
    shipText.setFillColor(Color::Black);
    shipText.setStyle(Text::Bold);
    FloatRect textBounds = shipText.getLocalBounds();
    shipText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    shipText.setPosition(shipPos.x, shipPos.y - 5);
    window.draw(shipText);
    Vector2f direction;
    if (bookingAnim.currentLeg >= 0 && bookingAnim.currentLeg < currentPath.size() - 1)
    {
        int from = currentPath.get(bookingAnim.currentLeg);
        int to = currentPath.get(bookingAnim.currentLeg + 1);
        direction = ports[to].position - ports[from].position;
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0)
            direction /= length;
    }
    for (int i = 0; i < 8; i++)
    {
        float offset = i * 4.0f;
        float size = 4.0f - i * 0.3f;
        float alpha = 80.0f - i * 8.0f;
        Vector2f wakePos = shipPos - direction * (10.0f + offset);
        wakePos.x += (rand() % 7 - 3);
        wakePos.y += (rand() % 7 - 3);
        CircleShape wake(size);
        wake.setPosition(wakePos - Vector2f(size, size));
        wake.setFillColor(Color(135, 206, 250, static_cast<Uint8>(alpha * pulse)));
        window.draw(wake);
    }
    if (bookingAnim.currentLeg >= 0 && bookingAnim.currentLeg < currentPath.size() - 1)
    {
        int from = currentPath.get(bookingAnim.currentLeg);
        int to = currentPath.get(bookingAnim.currentLeg + 1);
        Vector2f start = ports[from].position;
        Vector2f end = ports[to].position;
        Vertex completedLine[] = {
            Vertex(start, Color(50, 205, 50, 180)),
            Vertex(shipPos, Color(50, 205, 50, 180))};
        window.draw(completedLine, 2, Lines);
        Vertex remainingLine[] = {
            Vertex(shipPos, Color(135, 206, 235, 120)),
            Vertex(end, Color(135, 206, 235, 120))};
        window.draw(remainingLine, 2, Lines);
        float distance = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
        int segments = static_cast<int>(distance / 80.0f);
        for (int i = 1; i < segments; i++)
        {
            float t = i / static_cast<float>(segments);
            Vector2f markerPos = start + (end - start) * t;
            CircleShape marker(3.0f);
            marker.setPosition(markerPos - Vector2f(3, 3));
            marker.setFillColor(Color(255, 255, 255, 100));
            window.draw(marker);
        }
    }
}
inline void MaritimeSystem::drawBookingInfo(RenderWindow &window)
{
    if (!bookingAnim.active)
        return;
    RectangleShape infoPanel(Vector2f(320, 220));
    infoPanel.setFillColor(Color(0, 20, 40, 230));
    infoPanel.setOutlineColor(Color(255, 215, 0, 255));
    infoPanel.setOutlineThickness(3);
    infoPanel.setPosition(390, 600);
    window.draw(infoPanel);
    Text title("LIVE BOOKING TRACKER", font, 18);
    title.setFillColor(Color(255, 215, 0));
    title.setStyle(Text::Bold);
    title.setPosition(430, 616);
    window.draw(title);
    Text shipInfo("Ship: #" + to_string(bookingAnim.shipId), font, 16);
    shipInfo.setFillColor(Color::Cyan);
    shipInfo.setPosition(410, 640);
    window.draw(shipInfo);
    if (bookingAnim.currentLeg >= 0 && bookingAnim.currentLeg < currentPath.size() - 1)
    {
        int fromIdx = currentPath.get(bookingAnim.currentLeg);
        int toIdx = currentPath.get(bookingAnim.currentLeg + 1);
        Port &fromPort = ports[fromIdx];
        Port &toPort = ports[toIdx];
        string legText = "Leg " + to_string(bookingAnim.currentLeg + 1) + "/ " +
                         to_string(currentPath.size() - 1) + ":\n";
        legText += fromPort.name.c_str();
        legText += " → ";
        legText += toPort.name.c_str();
        Text legDisplay(legText, font, 14);
        legDisplay.setFillColor(Color::White);
        legDisplay.setPosition(410, 660);
        window.draw(legDisplay);
        Voyage *voyage = nullptr;
        Voyage *v = fromPort.voyages;
        while (v)
        {
            if (v->destinationPort == toIdx)
            {
                voyage = v;
                break;
            }
            v = v->next;
        }
        if (voyage != nullptr)
        {
            string details = "Cost: $" + to_string(voyage->cost) + "\n";
            int hours = voyage->getDurationMinutes() / 60;
            int mins = voyage->getDurationMinutes() % 60;
            details += "Time: " + to_string(hours) + "h " + to_string(mins) + "m\n";
            details += "Company: " + string(voyage->company.c_str());
            Text detailsText(details, font, 12);
            detailsText.setFillColor(Color(144, 238, 144));
            detailsText.setPosition(410, 704);
            window.draw(detailsText);
        }
    }
    float totalProgress = 0.0f;
    if (currentPath.size() > 1)
    {
        totalProgress = (bookingAnim.currentLeg + bookingAnim.progress) / (currentPath.size() - 1);
    }
    RectangleShape progressBg(Vector2f(280, 20));
    progressBg.setFillColor(Color::Yellow);
    progressBg.setPosition(410, 750);
    window.draw(progressBg);
    RectangleShape progressFill(Vector2f(280 * totalProgress, 20));
    progressFill.setFillColor(Color(0, 255, 127, 220));
    progressFill.setPosition(410, 750);
    window.draw(progressFill);
    string progressStr = "Journey Progress: " + to_string(static_cast<int>(totalProgress * 100)) + "%";
    Text progressText(progressStr, font, 14);
    progressText.setFillColor(Color::Black);
    progressText.setPosition(415, 750);
    window.draw(progressText);
    string stats = "Total Cost: $" + to_string(bookingAnim.totalCost) + "\n";
    int totalHours = bookingAnim.totalTime / 60;
    int totalMins = bookingAnim.totalTime % 60;
    stats += "Total Time: " + to_string(totalHours) + "h " + to_string(totalMins) + "m";
    Text statsText(stats, font, 13);
    statsText.setFillColor(Color(255, 182, 193));
    statsText.setPosition(415, 780);
    window.draw(statsText);
}
void MaritimeSystem::runTests()
{
    cout << "\n==================== RUNNING SYSTEM TESTS ====================\n";
    cout << "[TEST] MyString... ";
    MyString s1("Hello");
    MyString s2;
    if (strcmp(s1.c_str(), "Hello") == 0 && strcmp(s2.c_str(), "") == 0)
        cout << "PASS\n";
    else
        cout << "FAIL\n";
    cout << "[TEST] DateTime Logic... ";
    DateTime dt1(1, 1, 2024, 10, 0);
    DateTime dt2(2, 1, 2024, 10, 0);
    int diff = dt1.minutesDifference(dt2);
    if (diff == 1440)
        cout << "PASS (Difference: " << diff << "m)\n";
    else
        cout << "FAIL (Expected 1440, got " << diff << ")\n";
    cout << "[TEST] Data Loading... ";
    if (portCount > 0 && findPort("Karachi") != -1 && findPort("Singapore") != -1)
        cout << "PASS (Loaded " << portCount << " ports)\n";
    else
        cout << "FAIL (Ports not loaded correctly)\n";
    cout << "[TEST] Pathfinding (Karachi -> Singapore)... ";
    startPort = findPort("Karachi");
    endPort = findPort("Singapore");
    if (startPort != -1 && endPort != -1)
    {
        dijkstra(true);
        if (currentPath.size() > 0)
        {
            cout << "PASS (Found path with " << currentPath.size() << " nodes)\n";
            cout << "       Path: ";
            for (int i = 0; i < currentPath.size(); i++)
            {
                cout << ports[currentPath.get(i)].name.c_str();
                if (i < currentPath.size() - 1)
                    cout << " -> ";
            }
            cout << "\n";
        }
        else
        {
            cout << "FAIL (No path found)\n";
        }
    }
    else
    {
        cout << "FAIL (Ports not found)\n";
    }
    cout << "[TEST] Max Layover Logic... ";
    currentPath.clear();
    maxLayoverDays = 14;
    dijkstra(true);
    if (currentPath.size() > 0)
        cout << "PASS (Path found with 14 day limit)\n";
    else
        cout << "FAIL (No path with 14 day limit)\n";
    cout << "[TEST] Separate Ship Storage... ";
    int testPortIdx = findPort("Dubai");
    if (testPortIdx != -1)
    {
        ports[testPortIdx].addShip(101, MyString("MaerskLine"));
        ports[testPortIdx].addShip(102, MyString("MSC"));
        bool maerskFound = false;
        bool mscFound = false;
        for (int i = 0; i < ports[testPortIdx].companyQueues.size(); i++)
        {
            if (ports[testPortIdx].companyQueues.get(i).companyName == "MaerskLine")
            {
                if (!ports[testPortIdx].companyQueues.get(i).shipIds.empty())
                    maerskFound = true;
            }
            if (ports[testPortIdx].companyQueues.get(i).companyName == "MSC")
            {
                if (!ports[testPortIdx].companyQueues.get(i).shipIds.empty())
                    mscFound = true;
            }
        }
        if (maerskFound && mscFound)
            cout << "PASS (Ships stored in separate queues)\n";
        else
            cout << "FAIL (Ships not found in separate queues)\n";
    }
    else
    {
        cout << "FAIL (Port not found)\n";
    }
    cout << "==================== TESTS COMPLETED ====================\n\n";
}
#endif
