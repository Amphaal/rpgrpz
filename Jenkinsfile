pipeline {
    agent none
    stages {
        stage('Checkout') {
            agent any
            steps {
                sh 'git submodule update --init --recursive'
            }
        }
        stage('Configure && Build') {
            parallel {
                stage('MinGW') {
                    agent {
                        docker { image 'amphaal/rpgrpz-ci-windows' }
                    }
                    steps {
                        sh 'cmake -GNinja -B_genWindows -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-ci.cmake'
                        sh 'ninja -C B_genWindows'
                    }
                }
                stage('Linux') {
                    agent {
                        docker { image 'amphaal/rpgrpz-linux-ci' }
                    }
                    steps {
                        sh 'cmake -GNinja -B_genLinux -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux.cmake'
                        sh 'ninja -C B_genLinux'
                    }
                }
            }
        }
    }
}