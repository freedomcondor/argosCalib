vim argos_coding.sh -c "
	set ts=3
	set shiftwidth=3
	set expandtab

	tabnew /home/harry/code/argos3-builderbot/src/plugins/robots/block/simulator/dynamics3d_block_model.h
	vsp /home/harry/code/argos3-builderbot/src/plugins/robots/block/simulator/dynamics3d_block_model.cpp

	tabnew /home/harry/code/argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_model.h

   tabnew /home/harry/Desktop/code/argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_magnetism_plugin.h
   vsp /home/harry/Desktop/code/argos3-dev/src/plugins/simulator/physics_engines/dynamics3d/dynamics3d_magnetism_plugin.cpp

   tabnew /home/harry/Desktop/code/argos3-dev/src/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.cpp

   tabnew /home/harry/Desktop/code/argos3-builderbot/src/plugins/robots/builderbot/simulator/builderbot_electromagnet_system_entity.cpp

   tabnew /home/harry/Desktop/code/argos3-dev/src/plugins/robots/builderbot/simulator/dynamics3d_builderbot_model.cpp
"
