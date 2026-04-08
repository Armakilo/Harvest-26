
// Corey's Laser Functions

#include <xc.h>
#include "harvest.h"

void ShootShield()
{
    char tosend[6] = {0xFE, 0x19, 0x02, 0x09, 0x00, 0x00};
    sendit(tosend, 6);
}

void ShootAttack()
{
    char tosend[7] = {0xFE, 0x19, 0x01, 0x09, 0x01, 0x00, 1};
    sendit(tosend, 7); 
}

void ShootRepair()
{
    char tosend[6] = {0xFE, 0x19, 0x04, 0x09, 0x00, 0x00};
    sendit(tosend, 6);
}

void ShootLaser()
{
    if (SWD > 1600)
        {
            if (SWC > 1800)
            {
                if (!shield_code_flag)
                {
                    ShootShield();
                }
            }
            else if (SWC > 1300 && SWC < 1700)
            {
                ShootAttack();
            }
            else if (SWC < 200)
            {
                if (!repair_code_flag);
                {
                    ShootRepair();
                }
            }
        }
}
