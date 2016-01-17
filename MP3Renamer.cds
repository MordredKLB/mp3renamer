<distribution version="13.0.2" name="MP3Renamer" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{F3FC8BDD-7181-4FF1-8BD0-BC121409E4B0}">
		<general appName="MP3Renamer" outputLocation="d:\Source\MP3Renamer\cvidistkit.MP3Renamer_1" relOutputLocation="cvidistkit.MP3Renamer_1" outputLocationWithVars="d:\Source\MP3Renamer\cvidistkit.%name_1" relOutputLocationWithVars="cvidistkit.%name_1" upgradeBehavior="1" autoIncrement="true" version="1.2.1">
			<arp company="" companyURL="" supportURL="" contact="" phone="" comments=""/>
			<summary title="" subject="" keyWords="" comments="" author=""/></general>
		<userinterface language="English" showPaths="true" readMe="" license="">
			<dlgstrings welcomeTitle="MP3Renamer" welcomeText=""/></userinterface>
		<dirs appDirID="101">
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="MP3Renamer" dirID="100" parentID="7" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Program Files]" dirID="2" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="MP3Renamer" dirID="101" parentID="2" isMSIDir="false" visible="true" unlock="false"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="d:\Source\MP3Renamer\zlibDll\msvc\zlib1.lib" relSourcePath="zlibDll\msvc\zlib1.lib" relSourceBase="0" targetDir="101" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="1" sourcePath="d:\Source\MP3Renamer\cvibuild.mp3renamer\Release\MP3Renamer.exe" targetDir="101" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="101" projectID="0">
				<fileID>1</fileID></projectOutput></fileGroups>
		<shortcuts>
			<shortcut name="MP3Renamer" targetFileID="1" destDirID="100" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
		<mergemodules/>
		<products>
			<product name="NI LabWindows/CVI Shared Run-Time Engine" UC="{80D3D303-75B9-4607-9312-E5FC68E5BFD2}" productID="{D3C549FD-7DA5-440B-A1BC-DD92C898949A}" path="C:\ProgramData\National Instruments\MDF\ProductCache\" flavorID="_full_" flavorName="Full" verRestr="false" coreVer="">
				<cutSoftDeps>
					<softDep name=".NET Support" UC="{0DDB211A-941B-4125-9518-E81E10409F2E}" depKey="CVI_DotNet.CVI1320.RTE"/>
					<softDep name="ActiveX Container Support" UC="{1038A887-23E1-4289-B0BD-0C4B83C6BA21}" depKey="ActiveX_Container.CVI1320.RTE"/>
					<softDep name="Analysis Support" UC="{86208B51-159E-4F6F-9C62-0D5EFC9324D8}" depKey="CVI_Analysis.CVI1320.RTE"/>
					<softDep name="Low-Level Driver Support" UC="{20931852-6AA9-4918-926B-130D07A4CF8B}" depKey="CVI_LowLevelDriver.CVI1320.RTE"/>
					<softDep name="Network Streams Support" UC="{40A5AD7F-4BAF-4A5C-8B56-426B84F75C05}" depKey="CVI_NetworkStreams.CVI1320.RTE"/>
					<softDep name="Network Variable Support" UC="{15CE39FE-1354-484D-B8CA-459077449FB3}" depKey="CVI_NetworkVariable.CVI1320.RTE"/>
					<softDep name="Real-Time Utility Support" UC="{DA6BBF6F-0910-4860-AD46-F14DADF645E9}" depKey="Remote_Configuration.CVI1320.RTE"/>
					<softDep name="Visual C++ 2008 Redistributable" UC="{BD1E864A-2ACC-4F7E-9A7D-977C39952A52}" depKey="VC2008MSMs_Redist.CVI1320.RTE"/>
					<softDep name="Visual C++ 2010 Redistributable" UC="{3109870C-C165-4896-AC99-AFDD3CA3304A}" depKey="VC2010MSMs_Redist.CVI1320.RTE"/></cutSoftDeps></product></products>
		<runtimeEngine installToAppDir="false" activeXsup="false" analysis="false" cvirte="true" dotnetsup="false" instrsup="true" lowlevelsup="false" lvrt="true" netvarsup="false" rtutilsup="false">
			<hasSoftDeps/></runtimeEngine><sxsRuntimeEngine>
			<selected>false</selected>
			<doNotAutoSelect>false</doNotAutoSelect></sxsRuntimeEngine>
		<advanced mediaSize="650">
			<launchConditions>
				<condition>MINOS_WINXP</condition>
			</launchConditions>
			<includeConfigProducts>true</includeConfigProducts>
			<maxImportVisible>silent</maxImportVisible>
			<maxImportMode>merge</maxImportMode>
			<custMsgFlag>false</custMsgFlag>
			<custMsgPath>d:\bin\msgrte.txt</custMsgPath>
			<signExe>false</signExe>
			<certificate></certificate>
			<signTimeURL></signTimeURL>
			<signDescURL></signDescURL></advanced>
		<Projects NumProjects="1">
			<Project000 ProjectID="0" ProjectAbsolutePath="d:\Source\MP3Renamer\mp3renamer.prj" ProjectRelativePath="mp3renamer.prj"/></Projects>
		<buildData progressBarRate="2.918470810600452">
			<progressTimes>
				<Begin>0.000000000000000</Begin>
				<ProductsAdded>1.659880124999998</ProductsAdded>
				<DPConfigured>5.881756124999995</DPConfigured>
				<DPMergeModulesAdded>18.658007312499990</DPMergeModulesAdded>
				<DPClosed>23.664256999999985</DPClosed>
				<DistributionsCopied>24.139292187499979</DistributionsCopied>
				<End>34.264519500000006</End></progressTimes></buildData>
	</msi>
</distribution>
