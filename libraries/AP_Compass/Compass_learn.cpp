/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
#include "Compass.h"

// don't allow any axis of the offset to go above 2000
#define COMPASS_OFS_LIMIT 2000

/*
 * Correct rounding up and down with 0.5f as bias
 */
inline Vector3i round_vector3f(const Vector3f &v3f) {
  Vector3i v3i;
  v3i.x = fabs(v3f.x) < 0.5f ? 0 : v3f.x > 0.f ? v3f.x + 0.5f : v3f.x - 0.5f;
  v3i.y = fabs(v3f.y) < 0.5f ? 0 : v3f.y > 0.f ? v3f.y + 0.5f : v3f.y - 0.5f;
  v3i.z = fabs(v3f.z) < 0.5f ? 0 : v3f.z > 0.f ? v3f.z + 0.5f : v3f.z - 0.5f;
  return v3i;
}

/*
 *  this offset learning algorithm is inspired by this paper from Bill Premerlani
 *
 *  http://gentlenav.googlecode.com/files/MagnetometerOffsetNullingRevisited.pdf
 *
 *  The base algorithm works well, but is quite sensitive to
 *  noise. After long discussions with Bill, the following changes were
 *  made:
 *
 *   1) we keep a history buffer that effectively divides the mag
 *      vectors into a set of N streams. The algorithm is run on the
 *      streams separately
 *
 *   2) within each stream we only calculate a change when the mag
 *      vector has changed by a significant amount.
 *
 *  This gives us the property that we learn quickly if there is no
 *  noise, but still learn correctly (and slowly) in the face of lots of
 *  noise.
 */
void
Compass::learn_offsets(void)
{
    if (_learn == 0) {
        // auto-calibration is disabled
        return;
    }

    // this gain is set so we converge on the offsets in about 5
    // minutes with a 10Hz compass
    const float gain = 0.01;
    const float max_change = 10.0;
    const float min_diff = 50.0;
    
    if (!_null_init_done) {
        // first time through
        _null_init_done = true;
        for (uint8_t k=0; k<COMPASS_MAX_INSTANCES; k++) {
            const Vector3f &ofs = _state[k].offset.get();
            const Vector3f &field = _state[k].field;
            const Vector3f history = field - ofs;
          
            // fill the history buffer with the current mag vector,
            // with the offset removed
            for (uint8_t i=0; i<_mag_history_size; i++) {
                _state[k].mag_history[i] = round_vector3f(history);
            }
            _state[k].mag_history_index = 0;
        }
        return;
    }

    for (uint8_t k=0; k<COMPASS_MAX_INSTANCES; k++) {
        const Vector3f &ofs = _state[k].offset.get();
        const Vector3f &field = _state[k].field;
        Vector3f history = field - ofs;
      
        float length = 0.f;

        if (ofs.is_nan()) {
            // offsets are bad possibly due to a past bug - zero them
            _state[k].offset.set(Vector3f());
        }

        // get a past element
        Vector3f b1 = Vector3f( _state[k].mag_history[_state[k].mag_history_index].x,
                                _state[k].mag_history[_state[k].mag_history_index].y,
                                _state[k].mag_history[_state[k].mag_history_index].z );

        // the history buffer doesn't have the offsets
        b1 += ofs;

        // calculate the delta for this sample
        length = history.length();
        if (length < min_diff) {
            // the mag vector hasn't changed enough - we don't get
            // enough information from this vector to use it.
            // Note that we don't put the current vector into the mag
            // history here. We want to wait for a larger rotation to
            // build up before calculating an offset change, as accuracy
            // of the offset change is highly dependent on the size of the
            // rotation.
            _state[k].mag_history_index = (_state[k].mag_history_index + 1) % _mag_history_size;
            continue;
        }

        // put the vector in the history
        _state[k].mag_history[_state[k].mag_history_index] = round_vector3f(history);
        _state[k].mag_history_index = (_state[k].mag_history_index + 1) % _mag_history_size;

        // equation 6 of Bills paper
        history = history * (gain * (field.length() - b1.length()) / length);

        // limit the change from any one reading. This is to prevent
        // single crazy readings from throwing off the offsets for a long
        // time
        length = history.length();
        if (length > max_change) {
            history *= max_change / length;
        }

        Vector3f new_offsets = _state[k].offset.get() - history;

        if (new_offsets.is_nan()) {
            // don't apply bad offsets
            continue;
        }

        // constrain offsets
        new_offsets.x = constrain_float(new_offsets.x, -COMPASS_OFS_LIMIT, COMPASS_OFS_LIMIT);
        new_offsets.y = constrain_float(new_offsets.y, -COMPASS_OFS_LIMIT, COMPASS_OFS_LIMIT);
        new_offsets.z = constrain_float(new_offsets.z, -COMPASS_OFS_LIMIT, COMPASS_OFS_LIMIT);
            
        // set the new offsets
        _state[k].offset.set(new_offsets);
    }
}
