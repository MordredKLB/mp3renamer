<distribution version="13.0.0" name="MP3Renamer" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{3E0EB6C8-6C69-48FA-9EC7-F672F6A58653}">
		<general appName="MP3Renamer" outputLocation="c:\Program Files (x86)\National Instruments\CVI2012\testing\MP3Renamer\cvidistkit.MP3Renamer_1" relOutputLocation="cvidistkit.MP3Renamer_1" outputLocationWithVars="c:\Program Files (x86)\National Instruments\CVI2012\testing\MP3Renamer\cvidistkit.%name_1" relOutputLocationWithVars="cvidistkit.%name_1" upgradeBehavior="1" autoIncrement="true" version="1.0.0">
			<arp company="%company" companyURL="" supportURL="" contact="" phone="" comments=""/>
			<summary title="" subject="" keyWords="" comments="" author=""/></general>
		<userinterface language="English" showPaths="true" readMe="" license="">
			<dlgstrings welcomeTitle="MP3Renamer" welcomeText=""/></userinterface>
		<dirs appDirID="100">
			<installDir name="[Program Files]" dirID="2" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="MP3Renamer" dirID="100" parentID="2" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="MP3Renamer" dirID="101" parentID="7" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true" unlock="false"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="c:\Program Files (x86)\National Instruments\CVI2012\testing\MP3Renamer\cvibuild.mp3renamer\Release\MP3Renamer.exe" targetDir="100" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="100" projectID="0">
				<fileID>0</fileID></projectOutput>
			<projectDependencies dirID="100" projectID="0"/></fileGroups>
		<shortcuts>
			<shortcut name="MP3Renamer" targetFileID="0" destDirID="101" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
		<mergemodules/>
		<products>
			<product name="NI LabWindows/CVI Shared Run-Time Engine" UC="{80D3D303-75B9-4607-9312-E5FC68E5BFD2}" productID="" path="(None)" flavorID="_full_" flavorName="" verRestr="false" coreVer="1.0.0">
				<cutSoftDeps/></product></products>
		<runtimeEngine installToAppDir="false" activeXsup="true" analysis="true" cvirte="true" dotnetsup="true" instrsup="true" lowlevelsup="true" lvrt="true" netvarsup="true" rtutilsup="true">
			<hasSoftDeps/></runtimeEngine><sxsRuntimeEngine>
			<selected>false</selected>
			<doNotAutoSelect>false</doNotAutoSelect></sxsRuntimeEngine>
		<advanced mediaSize="650">
			<launchConditions>
				<condition>MINOS_WINXP_SP0</condition>
			</launchConditions>
			<includeConfigProducts>true</includeConfigProducts>
			<maxImportVisible>silent</maxImportVisible>
			<maxImportMode>merge</maxImportMode>
			<custMsgFlag>false</custMsgFlag>
			<custMsgPath>msgrte.txt</custMsgPath>
			<signExe>false</signExe>
			<certificate></certificate>
			<signTimeURL></signTimeURL>
			<signDescURL></signDescURL></advanced>
		<Projects NumProjects="1">
			<Project000 ProjectID="0" ProjectAbsolutePath="c:\Program Files (x86)\National Instruments\CVI2012\testing\MP3Renamer\mp3renamer.prj" ProjectRelativePath="mp3renamer.prj"/></Projects>
	</msi>
</distribution>
