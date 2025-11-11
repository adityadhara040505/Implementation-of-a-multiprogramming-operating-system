#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace std;

class PCB
{
public:
    int jobID, TTL, TLL;
    int TTC, LLC;
};

class OS2
{
private:
    fstream infile;
    ofstream outfile;
    char Memory[300][4], buffer[40], IR[4], R[4];
    PCB pcb;
    bool C;
    int IC;
    int SI, PI, TI;
    int PTR;
    int visited[30];
    bool mos;
    int RA, VA;
    bool Terminate;
    int PTE;
    int page_fault = 0;
    int pageNo;
    int PageTable_ptr;

    void INIT()
    {
        fill(buffer, buffer + sizeof(buffer), '\0');
        fill(&Memory[0][0], &Memory[0][0] + sizeof(Memory), '\0');
        for (int i = 0; i < 4; i++)
        {
            IR[i] = ' ';
            R[i] = ' ';
        }

        C = true;
        IC = 0;
        SI = PI = TI = 0;
        pcb.jobID = pcb.TLL = pcb.TTL = pcb.TTC = pcb.LLC = 0;
        PTR = PTE = pageNo = -1;
        for (int i = 0; i < 30; i++)
        {
            visited[i] = 0;
        }
        PageTable_ptr = 0;
        Terminate = false;
    }

    void BUFFER() { fill(buffer, buffer + sizeof(buffer), '\0'); }

    int ALLOCATE()
    {
        int pageNo;
        bool check = true;
        while (check)
        {
            pageNo = (rand() % 30);
            if (visited[pageNo] == 0)
            {
                visited[pageNo] = 1;
                check = false;
            }
        }
        return pageNo;
    }

    void MOS()
    {
        if (TI == 0 && SI == 1)
            READ();
        else if (TI == 0 && SI == 2)
            WRITE();
        else if (TI == 0 && SI == 3)
            TERMINATE(0);
        else if (TI == 2 && SI == 1)
            TERMINATE(3);
        else if (TI == 2 && SI == 2)
        {
            WRITE();
            TERMINATE(3);
        }
        else if (TI == 2 && SI == 3)
            TERMINATE(0);
        else if (TI == 0 && PI == 1)
            TERMINATE(4);
        else if (TI == 0 && PI == 2)
            TERMINATE(5);
        else if (TI == 0 && PI == 3)
        {
            if (page_fault == 1)
            {
                cout << " Page Fault:   ";
                pageNo = ALLOCATE();
                Memory[PTE][2] = (pageNo / 10) + '0';
                Memory[PTE][3] = (pageNo % 10) + '0';
                PageTable_ptr++;
                PI = 0;
                cout << "Allocated Page Number: " << pageNo << "\n";
            }
            else
            {
                pcb.TTC--;
                TERMINATE(6);
            }
        }
        else
        {
            pcb.TTC--;
            TERMINATE(3);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    void READ()
    {
    cout << "Read function called\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
        string data;
        getline(infile, data);
        if (data[0] == '$' && data[1] == 'E' && data[2] == 'N' && data[3] == 'D')
        {
            pcb.TTC--;
            TERMINATE(1);
            return;
        }

        int len = data.size();
        for (int i = 0; i < len; i++)
            buffer[i] = data[i];

        int buff = 0, mem_ptr = RA, end = RA + 10;
        while (buff < 40 && buffer[buff] != '\0' && mem_ptr < end)
        {
            for (int i = 0; i < 4; i++)
            {
                Memory[mem_ptr][i] = buffer[buff];
                buff++;
            }
            mem_ptr++;
        }
        BUFFER();
        SI = 0;
    }

    void WRITE()
    {
    cout << "Write function called\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
        pcb.LLC++;
        if (pcb.LLC > pcb.TLL)
        {
            pcb.LLC--;
            TERMINATE(2);
            return;
        }

        outfile.open("out.txt", ios::app);
        string output;

        if (RA != -1)
        {
            for (int i = RA; i < RA + 10; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (Memory[i][j] == '\0')
                        Memory[i][j] = ' ';
                    output += Memory[i][j];
                }
            }
            outfile << output << "\n";
        }
        SI = 0;
        outfile << "\n";
        outfile.close();
    }

    void TERMINATE(int EM)
    {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        Terminate = true;
        outfile.open("out.txt", ios::app);

        outfile << "Job Id :" << pcb.jobID << "\n";
        switch (EM)
        {
        case 0: outfile << "No Error: Program executed successfully\n"; break;
        case 1: outfile << "Error: Out of Data\n"; break;
        case 2: outfile << "Error: Line Limit Exceeded\n"; break;
        case 3: outfile << "Error: Time Limit Exceeded\n"; break;
        case 4: outfile << "Error: Operation Code Error\n"; break;
        case 5: outfile << "Error: Operand Error\n"; break;
        case 6: outfile << "Error: Invalid Page Fault\n"; break;
        }
        outfile << "IC: " << IC << "\nIR: ";
        for (int i = 0; i < 4; i++)
            if (IR[i] != '\0')
                outfile << IR[i];
        outfile << "\nTTC: " << pcb.TTC << "\nLLC: " << pcb.LLC << "\n\n\n";
        SI = PI = TI = 0;
        outfile.close();
    }

    void LOAD()
    {
        if (infile.is_open())
        {
            string s;
            while (getline(infile, s))
            {
                if (s[0] == '$' && s[1] == 'A' && s[2] == 'M' && s[3] == 'J')
                {
                    INIT();
                    cout << "New Job started\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(700));
                    pcb.jobID = (s[4] - '0') * 1000 + (s[5] - '0') * 100 + (s[6] - '0') * 10 + (s[7] - '0');
                    pcb.TTL = (s[8] - '0') * 1000 + (s[9] - '0') * 100 + (s[10] - '0') * 10 + (s[11] - '0');
                    pcb.TLL = (s[12] - '0') * 1000 + (s[13] - '0') * 100 + (s[14] - '0') * 10 + (s[15] - '0');

                    PTR = ALLOCATE() * 10;
                    for (int i = PTR; i < PTR + 10; i++)
                        for (int j = 0; j < 4; j++)
                            Memory[i][j] = '*';
                    cout << "\nAllocated Page is for Page Table: " << PTR / 10 << "\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    cout << "jobID: " << pcb.jobID << "\nTTL: " << pcb.TTL << "\nTLL: " << pcb.TLL << "\n";
                }

                else if (s.substr(0, 4) == "$DTA")
                {
                    cout << "Data card loading\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(700));
                    BUFFER();
                    STARTEXECUTION();
                }

                else if (s.substr(0, 4) == "$END")
                {
                    cout << "END of Job\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }

                else
                {
                    BUFFER();
                    pageNo = ALLOCATE();
                    Memory[PTR + PageTable_ptr][2] = (pageNo / 10) + '0';
                    Memory[PTR + PageTable_ptr][3] = (pageNo % 10) + '0';
                    PageTable_ptr++;

                    cout << "Program Card loading\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    cout << "Allocated pageno. for prog card=" << pageNo << "\n";
                    int length = s.size();

                    for (int i = 0; i < length; i++)
                        buffer[i] = s[i];

                    int buff = 0;
                    IC = pageNo * 10;
                    int end = IC + 10;

                    while (buff < 40 && buffer[buff] != '\0' && IC < end)
                    {
                        for (int j = 0; j < 4; j++)
                        {
                            if (buffer[buff] == 'H')
                            {
                                Memory[IC][j] = 'H';
                                buff++;
                                break;
                            }
                            Memory[IC][j] = buffer[buff];
                            buff++;
                        }
                        IC++;
                    }
                }
            }
            infile.close();
        }
    }

    int ADDRESSMAP(int VA)
    {
        if (0 <= VA && VA < 100)
        {
            PTE = PTR + (VA / 10);
            if (Memory[PTE][2] == '*')
            {
                PI = 3;
                cout << "\npage fault occurred";
                MOS();
            }
            else
            {
                string p;
                p = Memory[PTE][2];
                p += Memory[PTE][3];
                int pageNo = stoi(p);
                RA = pageNo * 10 + (VA % 10);
                cout << "\nreturned RA= " << RA;
                return RA;
            }
        }
        else
        {
            PI = 2;
            cout << "\noperand error called";
            MOS();
        }
        return pageNo * 10;
    }

    void STARTEXECUTION()
    {
        IC = 0;
        EXECUTEUSERPROGRAM();
    }

    void EXECUTEUSERPROGRAM()
    {
        while (!Terminate)
        {
            mos = false;
            RA = ADDRESSMAP(IC);
            if (PI != 0)
                return;

            for (int i = 0; i < 4; i++)
                IR[i] = Memory[RA][i];

            IC++;

            string op;
            op += IR[2];
            op += IR[3];
            cout << "\ninstruct = " << IR[0] << IR[1] << IR[2] << IR[3];
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            if (IR[0] == 'G' && IR[1] == 'D')
            {
                SIMULATION();
                page_fault = 1;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                    PI = 2, mos = true;
                else
                {
                    VA = stoi(op);
                    cout << "\nvirtual address= " << VA;
                    RA = ADDRESSMAP(VA);
                    SI = 1;
                    mos = true;
                }
            }

            else if (IR[0] == 'P' && IR[1] == 'D')
            {
                SIMULATION();
                page_fault = 0;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                    PI = 2, pcb.TTC--, mos = true;
                else
                {
                    SI = 2;
                    VA = stoi(op);
                    RA = ADDRESSMAP(VA);
                    mos = true;
                }
            }

            else if (IR[0] == 'H')
            {
                SIMULATION();
                SI = 3;
                mos = true;
                Terminate = true;
            }
            else
            {
                SIMULATION();
                PI = 1;
                SI = 0;
                pcb.TTC--;
                mos = true;
            }

            if (mos)
            {
                cout << "\nMOS called for- " << IR[0] << IR[1] << IR[2] << IR[3];
                MOS();
            }
        }
    }

    void SIMULATION()
    {
        pcb.TTC++;
        cout << "\nTTC= " << pcb.TTC;
        if (pcb.TTC > pcb.TTL)
        {
            TI = 2;
            cout << "\nTime limit exceeded";
        }
    }

public:
    OS2()
    {
        infile.open("x.txt", ios::in);
        INIT();
        LOAD();
    }
};

int main()
{
    OS2 os;
    return 0;
}
