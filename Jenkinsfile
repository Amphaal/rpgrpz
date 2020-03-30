pipeline {
    agent none
    stages {
        stage('Configure && Build') {
            parallel {
                stage('MinGW') {
                    agent {
                        docker { image 'amphaal/rpgrpz-ci-windows' }
                    }
                    steps {
                        sh 'cmake -GNinja -B_genWindows -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-ci.cmake'
                        sh 'cmake --build ./_genWindows --target zipForDeploy'
                        withCredentials([usernameColonPassword(credentialsId: 'jenkins-bintray-api-key', variable: 'BINTRAY_API_KEY')]) {
                            sh 'curl -T _genWindows/installer.zip -uamphaal:$BINTRAY_API_KEY https://api.bintray.com/content/amphaal/rpgrpz/install-packages/latest/win64?override=1?publish=1?explode=1'
                            sh 'curl -T _genWindows/repository.zip -uamphaal:$BINTRAY_API_KEY https://api.bintray.com/content/amphaal/rpgrpz/ifw/latest/ifw-win64?override=1?publish=1?explode=1'
                        }
                    }
                }
                // stage('Linux') {
                //     agent {
                //         docker { image 'amphaal/rpgrpz-linux-ci' }
                //     }
                //     steps {
                //         sh 'cmake -GNinja -B_genLinux -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-ci.cmake'
                //         sh 'ninja -C _genLinux'
                //     }
                // }
            }
        }
    }
}