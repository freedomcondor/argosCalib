vim coding_argos.sh -c "
	set ts=3
	set shiftwidth=3
	set expandtab

	tabnew argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_model.h
   vsp argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_virtual_magnetism_plugin.cpp

   tabnew argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_virtual_magnetism_plugin.h

   tabnew argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_magnetism_plugin.h
   vsp argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_magnetism_plugin.cpp

   tabnew argos3-dev/src/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.cpp

   tabnew argos3-builderbot/src/plugins/robots/builderbot/simulator/builderbot_electromagnet_system_entity.cpp
"

#	tabnew argos3-builderbot/src/plugins/robots/block/simulator/dynamics3d_block_model.h
#	vsp argos3-builderbot/src/plugins/robots/block/simulator/dynamics3d_block_model.cpp

#   tabnew argos3-dev/src/plugins/robots/builderbot/simulator/dynamics3d_builderbot_model.h
#   vsp argos3-dev/src/plugins/robots/builderbot/simulator/dynamics3d_builderbot_model.cpp

#	tabnew argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_model.h
