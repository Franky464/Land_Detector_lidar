// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
  additional arming checks for plane
 */
#include "Plane.h"

const AP_Param::GroupInfo AP_Arming_Plane::var_info[] PROGMEM = {
    // variables from parent vehicle
    AP_NESTEDGROUPINFO(AP_Arming, 0),

    // @Param: RUDDER
    // @DisplayName: Rudder Arming
    // @Description: Control arm/disarm by rudder input. When enabled arming is done with right rudder, disarming with left rudder. Rudder arming only works in manual throttle modes with throttle at zero
    // @Values: 0:Disabled,1:ArmingOnly,2:ArmOrDisarm
    // @User: Advanced
    AP_GROUPINFO("RUDDER",       3,     AP_Arming_Plane,  rudder_arming_value,     ARMING_RUDDER_ARMONLY),

    AP_GROUPEND
};


/*
  additional arming checks for plane
 */
bool AP_Arming_Plane::pre_arm_checks(bool report)
{
    // call parent class checks
    bool ret = AP_Arming::pre_arm_checks(report);

    // Check airspeed sensor
    ret &= AP_Arming::airspeed_checks(report);

    if (plane.g.roll_limit_cd < 300) {
        if (report) {
            gcs_send_text_P(SEVERITY_HIGH,PSTR("PreArm: LIM_ROLL_CD too small"));        
        }
        ret = false;        
    }

    if (plane.aparm.pitch_limit_max_cd < 300) {
        if (report) {
            gcs_send_text_P(SEVERITY_HIGH,PSTR("PreArm: LIM_PITCH_MAX too small"));        
        }
        ret = false;        
    }

    if (plane.aparm.pitch_limit_min_cd > -300) {
        if (report) {
            gcs_send_text_P(SEVERITY_HIGH,PSTR("PreArm: LIM_PITCH_MIN too large"));        
        }
        ret = false;        
    }

    if (plane.channel_throttle->get_reverse() && 
        plane.g.throttle_fs_enabled &&
        plane.g.throttle_fs_value < 
        plane.channel_throttle->radio_max) {
        if (report) {
            gcs_send_text_P(SEVERITY_HIGH,PSTR("PreArm: invalid THR_FS_VALUE for rev throttle"));        
        }
        ret = false;
    }

    return ret;
}
