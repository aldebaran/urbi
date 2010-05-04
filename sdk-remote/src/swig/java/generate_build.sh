#!/bin/sh
# This script print the sample build.xml file on its output. Use it to
# generate sample/build.xml.


output_dir=sample
program_names=$(find . -name "*.java"					\
    -exec grep "public static .* main" "{}" \;				\
    -exec grep "public[ ]\+class" "{}" \;				\
    -exec echo "** {} ##" \;						\
    | grep -v "static"							\
    | sed 's/public[ ]\+class[ \t]*\([A-Za-z]\+\).*/\1/g'		\
    | tr -d '\n' | tr -d ' ' | sed -e 's/##/\n/g' | sed -e 's/\*\*/+/g')

libname=liburbijava
project_name=liburbi
compile_default=examples
exampledir=./examples/
srcdir=./liburbi/
libdir=.
classesdir=./classes
javadocdir=./doc/html/
jarpath=./lib
distdir=./dist/


## ------------------------- ##
## Header and main variables ##
## ------------------------- ##

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!--
   Copyright 2002-2004 The Apache Software Foundation

   Licensed under the Apache License, Version 2.0 (the \"License\");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an \"AS IS\" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
-->


<project default=\"$compile_default\" name=\"$project_name\" basedir=\".\">
  <property name=\"exampledir\" value=\"$exampledir\">
  </property>
  <property name=\"srcdir\" value=\"$srcdir\">
  </property>
  <property name=\"libdir\" value=\"$libdir\">
  </property>
  <property name=\"classesdir\" value=\"$classesdir\">
  </property>
  <property name=\"javadocdir\" value=\"$javadocdir\">
  </property>
  <property name=\"jarpath\" value=\"$jarpath\">
  </property>
  <property name=\"distdir\" value=\"$distdir\">
  </property>
  <property name=\"name\" value=\"$libname\">
  </property>
"


## -------------------------- ##
## Example projects variables ##
## -------------------------- ##

for couple in $program_names; do
    name=$(echo $couple | cut -d '+' -f 1)
    dir=$(basename $(dirname $(echo $couple | cut -d '+' -f 2)))
    echo "
  <property name=\"exampledir-$name\" value=\"\${exampledir}/$dir\">
  </property>
  <property name=\"manifestpath-$name\" value=\"\${exampledir}/$dir/manifest\">
  </property>
  <property name=\"builddir-$name\" value=\"\${exampledir-$name}/\">
  </property>
  <property name=\"name-$name\" value=\"$name\">
  </property>"
done


## -------------------- ##
## Compile liburbi java ##
## -------------------- ##

echo "
  <target name=\"compile\" description=\"* Compile the code\">
    <mkdir dir=\"\${classesdir}\">
    </mkdir>
    <javac destdir=\"\${classesdir}\" deprecation=\"true\" debug=\"true\" optimize=\"false\">
      <src>
         <pathelement location=\"\${srcdir}/image/\">
         </pathelement>
         <pathelement location=\"\${srcdir}/sound/\">
         </pathelement>
         <pathelement location=\"\${srcdir}/main/\">
         </pathelement>
      </src>
      <classpath>
        <fileset dir=\"\${libdir}\">
          <include name=\"*.jar\">
          </include>
        </fileset>
      </classpath>
    </javac>
  </target>
  <target name=\"jar\" description=\"* Create the jar\" depends=\"compile\">
    <jar destfile=\"\${jarpath}/${name}.jar\" basedir=\"\${classesdir}\">
    </jar>
  </target>
"

## ---------------- ##
## Compile examples ##
## ---------------- ##

for couple in $program_names; do
    name=$(echo $couple | cut -d '+' -f 1)
    dir=$(basename $(dirname $(echo $couple | cut -d '+' -f 2)))
    echo "
  <target name=\"compile-$name\" description=\"* Compile the $name example\" depends=\"jar\">
    <mkdir dir=\"\${builddir-$name}/classes\">
    </mkdir>
    <javac destdir=\"\${builddir-$name}/classes\" deprecation=\"true\" debug=\"true\" optimize=\"false\">
      <src>
         <pathelement location=\"\${exampledir-$name}\">
        </pathelement>
      </src>
      <classpath>
        <fileset dir=\"\${libdir}\">
          <include name=\"*.jar\">
          </include>
        </fileset>
      <pathelement path=\"\${classesdir}\">
      </pathelement>
     </classpath>
   </javac>
  </target>
  <target name=\"jar-$name\" description=\"* Create the jar of the $name example\" depends=\"compile-$name\">
    <jar destfile=\"\${builddir-$name}/\${name-$name}.jar\" manifest=\"\${manifestpath-$name}\">
       <fileset dir=\"\${classesdir}\">
        </fileset>
       <fileset dir=\"\${builddir-$name}/classes\">
        </fileset>
    </jar>
  </target>
"

done




echo -n "<target name=\"examples\" description=\"* Create the jars of the examples\" depends=\""
list=""
for couple in $program_names; do
    name=$(echo $couple | cut -d '+' -f 1)
    list="${list}jar-$name, "
done
echo -n $list | sed 's/,$//'
echo "\">
  </target>

  <target name=\"clean\" description=\"* Clean up the generated directories\">
	<delete>
		<fileset dir=\"\${jarpath}\" includes=\"**/*~\"/>
	</delete>

	<delete file=\"\${jarpath}/\${name}.jar\">
	</delete>"

for couple in $program_names; do
    name=$(echo $couple | cut -d '+' -f 1)
    echo "	<delete file=\"\${exampledir-$name}/\${name-$name}.jar\">
	</delete>"
done

echo "    <delete dir=\"\${classesdir}\">
    </delete>"

for couple in $program_names; do
    name=$(echo $couple | cut -d '+' -f 1)
    echo "    <delete dir=\"\${exampledir-$name}/classes\">
    </delete>"
done

echo "  </target>

  <target name=\"dist\" description=\"* Create a distribution\" depends=\"jar, doc\">
    <mkdir dir=\"\${distdir}\">
    </mkdir>
    <copy todir=\"\${distdir}\">
      <fileset dir=\"\${jarpath}\" includes=\"*.jar\">
      </fileset>
      <fileset dir=\"\${libdir}\" includes=\"*.jar\">
      </fileset>
      <fileset dir=\".\" includes=\"LICENSE*, NOTICE*, README*\">
      </fileset>
    </copy>
  </target>

  <target name=\"doc\" description=\"* Generate javadoc\">
    <mkdir dir=\"\${javadocdir}\">
    </mkdir>
    <property name=\"copyright\" value=\"Copyright URBI Project. All Rights Reserved.\">
    </property>
    <property name=\"title\" value=\"Liburbi Java\">
    </property>
    <javadoc use=\"true\" private=\"true\" destdir=\"\${javadocdir}\" author=\"true\" version=\"true\" sourcepath=\"java\" packagenames=\"liburbi.*\">
      <classpath>
        <fileset dir=\"\${libdir}\">
          <include name=\"*.jar\">
          </include>
        </fileset>
        <pathelement location=\"\${jarpath}/\${name}.jar\">
        </pathelement>
      </classpath>
    </javadoc>
  </target>
</project>"
