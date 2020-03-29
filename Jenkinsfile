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
                        sh 'ninja -C _genWindows'
                        sh 'cmake --build ./_genWindows --target zipForDeploy'
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