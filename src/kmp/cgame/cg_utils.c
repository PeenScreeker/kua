/*
  ==============================
  Written by:
    id software :            Quake III Arena
    nightmare, hk, Jelvan1 : mdd cgame Proxymod
    sOkam! :                 Kua: Multiplayer

  This file is part of Kua: Multiplayer.

  Kua: Multiplayer is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Kua: Multiplayer is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Kua: Multiplayer.  If not, see <http://www.gnu.org/licenses/>.
  ==============================
*/
// #include "cg_utils.h"  //Moved to cg_local.h

#include "cg_local.h"
// #include "cg_vm.h"     //#TODO: We don't need this, I imagine
// #include "defrag.h"    //#TODO: Remove and reconnect PS correctly

// TODO: Remove this when hud is ported. Not used. We have direct access to the data
// snapshot_t const* getSnap(void) {
//   static snapshot_t snapshot;
//   int32_t           curSnapNum;
//   int32_t           servertime;
//   trap_GetCurrentSnapshotNumber(&curSnapNum, &servertime);
//   trap_GetSnapshot(curSnapNum, &snapshot);
//   return &snapshot;
// }
//::::::::::
// playerState_t const* getPs(void) {
//   //TODO: Fix this
//   //      Second option shouldn't be needed, since we are not proxying the vm
//   // if (cvar_getInteger("g_synchronousClients")) {return &getSnap()->ps;}
//   // return (playerState_t const*)VM_ArgPtr(defrag()->pps_offset);
//   return &getSnap()->ps;
// }
//::::::::::
