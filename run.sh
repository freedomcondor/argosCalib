cd argos3-dev

argosBuildDir="build"

#this if may not be necessary
#if [ "$ARGOS_PLUGIN_PATH" == "" ]; then 
	echo "sourcing"
	source $argosBuildDir/setup_env.sh
#fi

#$argosBuildDir/core/argos3 -c experiment_share_clustering.argos
#$argosBuildDir/core/argos3 -c experiment_cmd_clustering.argos
#$argosBuildDir/core/argos3 -c experiment_surround.argos
#$argosBuildDir/core/argos3 -c experiment_connection.argos
$argosBuildDir/core/argos3 -c src/testing/test_builderbot.argos

cd ..
