pipeline {
    agent none
    stages {
        stage('Configure && Build') {
            parallel {
                stage('MinGW') {
                    agent {
                        docker { image 'amphaal/rpgrpz-ci-windows' }
                    }
                    environment {
                        BINTRAY_API_KEY = credentials('jenkins-bintray-api-key')
                    }
                    steps {
                        sh 'cmake -GNinja -B_genWindows -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-ci.cmake'
                        sh 'cmake --build ./_genWindows --target zipForDeploy'
                        sh 'curl -T _genWindows/installer.zip -uamphaal:$BINTRAY_API_KEY https://api.bintray.com/content/amphaal/rpgrpz/rpgrpz-win/latest'
                        sh 'curl -T _genWindows/repository.zip -uamphaal:$BINTRAY_API_KEY https://api.bintray.com/content/amphaal/rpgrpz/rpgrpz-ifw-win/latest'
                    }
                }
                stage('Linux') {
                    agent {
                        docker { image 'amphaal/rpgrpz-linux-ci' }
                    }
                    steps {
                        sh 'cmake -GNinja -B_genLinux -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-ci.cmake'
                        sh 'ninja -C _genLinux'
                    }
                }
            }
        }
    }
}