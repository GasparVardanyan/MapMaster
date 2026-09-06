## TODOs
- [ ] fix the smart ptr vs reference mess
- [ ] resource overlap behaviour
- [x] better camera controller
- [ ] parse occluders
- [x] R3D backend for PropGPUResourceManager
- [ ] R3D map rendering - half done
- [ ] imgui docking
- [ ] rlImgui

## BUGs
- [ ] CPU resource manager makes use of loaded mesh and texture resources after
processing them and so prevents the use of non collecting ParallelTask
