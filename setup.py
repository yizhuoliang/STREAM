from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext
import subprocess
import os
import sys
import platform

class StreamBuild(build_ext):
    """Custom build command for STREAM benchmark C code."""
    
    def run(self):
        # Navigate to the C source directory
        c_src_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pystream', 'c_src')
        
        # Store the current directory
        cwd = os.getcwd()
        
        try:
            # Change to the C source directory
            os.chdir(c_src_dir)
            
            # Run make to build the C program
            subprocess.check_call(['make', 'clean'])
            
            # Build with standard configuration
            subprocess.check_call(['make'])
            
            # Move back to the original directory
            os.chdir(cwd)
            
            # Continue with the regular build_ext process
            build_ext.run(self)
            
        except Exception as e:
            print(f"Error building STREAM benchmark: {e}")
            os.chdir(cwd)  # Ensure we return to the original directory
            raise

setup(
    name="pystream",
    version="0.1.0",
    description="Python wrapper for STREAM memory bandwidth benchmark",
    author="Your Name",
    author_email="your.email@example.com",
    packages=find_packages(),
    include_package_data=True,
    package_data={
        'pystream': ['c_src/*'],
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
    ],
    python_requires=">=3.6",
    cmdclass={
        'build_ext': StreamBuild,
    },
    install_requires=[
        'psutil',  # For process management
    ],
)
